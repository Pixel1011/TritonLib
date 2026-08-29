#include "TritonController.h"
#include "Constants.h"
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

TritonController::TritonController(hid_device* handle, ETritonPairType connection) {
  this->hid_handle = handle;
  this->pairType = connection;
  this->pendingInputUpdates.reserve(64);
}

TritonController::~TritonController() {
  close();
}

void TritonController::close() {
  this->stopPoll();
  if (pollThread.joinable()) {
    pollThread.join();
  }
  if (hid_handle != nullptr) {
    hid_close(hid_handle);
    hid_handle = nullptr;
  }
}

// these 2 are basically the only thing stolen from SteamHapticsSinger
int TritonController::playNote(int channel, int note, int gaindb) {
  double frequency = midiFrequency[note];
  return playFrequency(channel, frequency, gaindb, 0xffff);
}

// packet builder, idk if i should have these for all of them
int TritonController::playFrequency(uint8_t channel, uint16_t frequency, int8_t gaindb, uint16_t durationms, uint16_t lfoFreq, uint8_t lfoDepth) {
  MsgHapticLfoTone packet{};

  if (frequency == -1) {
    packet.side = channel;
  } else {
    int frequencyValue = static_cast<uint16_t>(frequency);
    packet.side = channel;
    packet.gain_db = gaindb;
    packet.frequency = frequencyValue;
    packet.duration_ms = durationms;
    packet.lfo_freq = 0;
    packet.lfo_depth = 0;
  }

  return sendLFOTone(&packet);
}

int TritonController::_playStereoAudio(uint8_t pcmBytes[], size_t length, TritonPCMMode mode, std::function<void(int step, int* readPointer)> callback) {
  if (length <= 0) return -1;
  // not enough bandwidth so cant play regardless
  if (this->pairType == ETritonPairType::k_ETritonPairType_Wireless && mode == TritonPCMMode::Khz8_16Bit) return -2;

  this->setupPCMStreaming(mode);

  int SAMPLE_RATE = 8000;
  int BYTES_PER_FRAME = 2;
  int SAMPLES_PER_PACKET = 31;
  int bytesPerChannel = 31;

  if (mode == TritonPCMMode::Khz1_16Bit || mode == TritonPCMMode::Khz2_16Bit || mode == TritonPCMMode::Khz4_16Bit || mode == TritonPCMMode::Khz8_16Bit) {
    BYTES_PER_FRAME = 4;
    SAMPLES_PER_PACKET = 15;
    bytesPerChannel = 30;
  }

  size_t NEED_BYTES = SAMPLES_PER_PACKET * BYTES_PER_FRAME;
  // clang-format off
  if (mode == TritonPCMMode::Khz1_16Bit || mode == TritonPCMMode::Khz1_8Bit || mode == TritonPCMMode::Khz1_8Bit_ulaw) {
    SAMPLE_RATE = 1000;
  } else
  if (mode == TritonPCMMode::Khz2_16Bit || mode == TritonPCMMode::Khz2_8Bit || mode == TritonPCMMode::Khz2_8Bit_ulaw) {
    SAMPLE_RATE = 2000;
  } else
  if (mode == TritonPCMMode::Khz4_16Bit || mode == TritonPCMMode::Khz4_8Bit || mode == TritonPCMMode::Khz4_8Bit_ulaw) {
    SAMPLE_RATE = 4000;
  } else
  if (mode == TritonPCMMode::Khz8_16Bit || mode == TritonPCMMode::Khz8_8Bit || mode == TritonPCMMode::Khz8_8Bit_ulaw) {
    SAMPLE_RATE = 8000;
  }
  // clang-format on
  auto period = std::chrono::microseconds((SAMPLES_PER_PACKET * 1000000) / SAMPLE_RATE);
  int readPointer = 0;

  MsgHapticPCMStereo packet;

  auto nextPacketTime = std::chrono::steady_clock::now();
  {
    std::scoped_lock lock(audioMutex);
    playingAudio.store(true);
  }
  while (true) {
    if (!playingAudio.load()) return 0;
    // yes this intentionally skips the first 60-62 bytes
    readPointer += NEED_BYTES;
    uint8_t tmp[NEED_BYTES]{0};

    if (readPointer >= static_cast<int>(length)) break;

    // read bytes
    int r = std::min(static_cast<int>(length) - readPointer, static_cast<int>(NEED_BYTES));
    std::memcpy(tmp, pcmBytes + readPointer, r);
    if (r < static_cast<int>(NEED_BYTES)) std::memset(tmp + r, 0, NEED_BYTES - r); // s8 silence = 0

    packet.length = bytesPerChannel;

    for (int i = 0; i < SAMPLES_PER_PACKET; i++) {
      if (BYTES_PER_FRAME == 4) {
        // 60 bytes over 15 samples
        // 16 bit
        size_t base = i * 4;
        uint8_t leftLow = tmp[base];
        uint8_t leftHigh = tmp[base + 1];
        uint8_t rightLow = tmp[base + 2];
        uint8_t rightHigh = tmp[base + 3];

        packet.left[i * 2] = leftLow;
        packet.left[i * 2 + 1] = leftHigh;
        packet.right[i * 2] = rightLow;
        packet.right[i * 2 + 1] = rightHigh;
      } else {
        // 8 bit
        // 62 bytes over 31 samples
        uint8_t left = tmp[i * 2];
        uint8_t right = tmp[i * 2 + 1];
        packet.left[i] = left;
        packet.right[i] = right;
      }
    }

    this->sendPCMStereo(&packet);
    callback(NEED_BYTES, &readPointer);
    nextPacketTime += period;

    while (std::chrono::steady_clock::now() < nextPacketTime) {}
  }

  return 0;
}

int TritonController::playStereoAudio(uint8_t pcmBytes[], size_t length, TritonPCMMode mode, std::function<void(int step, int* readPointer)> callback) {
  {
    std::scoped_lock lock(audioMutex);
    if (playingAudio.load()) {
      playingAudio.store(false);
    }
  }
  if (playThread.joinable()) playThread.join();

  playThread = std::thread([this, pcmBytes, length, mode, callback = std::move(callback)]() {
    _playStereoAudio(pcmBytes, length, mode, callback);
  });

  return 1;
}

void TritonController::setLizardMode(LizardModeState_t mode) {
  // disable SETTING_STEAM_WATCHDOG_ENABLE to stop lizard mode resetting then actually turn off lizard mode, or vice versa
  // thamks linux kernel for telling me how to do this without needing to use a thread to constantly send them every 3 secs (at least i think after looking at the code for a few mins)
  this->setControllerSetting(SETTING_STEAM_WATCHDOG_ENABLE, mode);
  this->setControllerSetting(SETTING_LIZARD_MODE, mode);
}

int TritonController::sendPCMMode(MsgHapticPCMMode* packet) {
  constexpr size_t size = sizeof(MsgHapticPCMMode);

  unsigned char buff[size + 1];
  buff[0] = PCM_MODE;
  memcpy(&buff[1], packet, size);
  return sendRaw(buff, sizeof(buff));
}

int TritonController::sendPCMStereo(MsgHapticPCMStereo* packet) {
  constexpr size_t size = sizeof(MsgHapticPCMStereo);

  unsigned char buff[size + 1] = {0};
  buff[0] = PCM_STEREO;
  memcpy(&buff[1], packet, size);
  return sendRaw(buff, sizeof(buff));
}

int TritonController::sendLFOTone(MsgHapticLfoTone* packet) {

  constexpr size_t size = sizeof(MsgHapticLfoTone);

  unsigned char buff[size + 1] = {0};
  buff[0] = LFO_TONE;
  memcpy(&buff[1], packet, size);
  return sendRaw(buff, sizeof(buff));
}

// will return -1 if controller disconnects, otherwise length of bytes read
int TritonController::sendRaw(uint8_t packet[], size_t length) {
  int r = hid_write(this->hid_handle, packet, length);
  if (r < 0) {
    printf("Send Error, hid_error: %ls\n", hid_error(this->hid_handle));
    stopPoll();
    disconnected.store(true);
    return -1;
  }
  return r;
}

int TritonController::sendFeatureReport(FeatureReportMsg* msg, size_t length) {
  uint8_t buff[length + 1] = {0};
  buff[0] = 1;
  memcpy(&buff[1], msg, length);

  int r = hid_send_feature_report(this->hid_handle, buff, length + 1);
  if (r < 0) {
    printf("Feature report send error: %ls\n", hid_error(this->hid_handle));
    stopPoll();
    disconnected.store(true);
    return -1;
  }
  return r;
}

int TritonController::setControllerSetting(ControllerSettings setting, uint16_t value) {
  if (setting < 0 || setting > SETTING_COUNT) return -4;

  FeatureReportMsg settingsMsg{};
  settingsMsg.header.type = ID_SET_SETTINGS_VALUES;
  settingsMsg.header.length = sizeof(ControllerSetting);
  settingsMsg.payload.setSettingsValues.settings[0].settingNum = setting;
  settingsMsg.payload.setSettingsValues.settings[0].settingValue = value;
  return sendFeatureReport(&settingsMsg, sizeof(settingsMsg));
}

// will return -1 if controller disconnects, otherwise length of bytes read
int TritonController::readRaw(uint8_t buff[], size_t length) {
  int r = hid_read_timeout(this->hid_handle, buff, length, 100);
  if (r < 0 || (pairType == ETritonPairType::k_ETritonPairType_Wireless && r <= 0)) {
    printf("Read Error, hid_error: %ls\n", hid_error(this->hid_handle));
    stopPoll();
    disconnected.store(true);
    return -1;
  }
  return r;
}

TritonMTUFull_t TritonController::getFullReport() {
  std::scoped_lock lock(stateMutex);
  return _state;
}

TritonBatteryStatus_t TritonController::getBatteryStatus() {
  std::scoped_lock lock(batteryMutex);
  return _battery;
}

// use input queue instead, but will leave this 
bool TritonController::isPressed(TritonButtons btn) {
  std::scoped_lock lock(stateMutex);
  return (_state.buttons & btn) != 0;
}

void TritonController::setupPCMStreaming(TritonPCMMode mode) {
  // old implementation
  /*//std::cout << "Running setup for pcm streaming! You may hear some weird noises" << std::endl;
  // not entirely sure how these 2 actually make the pcm streaming work
  // channels:
  // 0 - only left audio plays
  // 1 - only right audio plays
  // 3-5 on their own, nothing plays
  // but then 0,3,4,5 seems to sound okay??
  // 0xff param i believe causes the steam controller firmware to crash (bitmask?)
  // idk im just leaving it at this to not risk audio quality, which they do seem to affect from testing
  // only needs to be setup once though per restart of the controller. then any pcm streamed to it will play just fine

  // new imp from data by iczero (ty :D)

  // play on all actuators
  // i would make this an enum but i still think its wrong, 0,1 is clearly playing on internal haptics
  // 3,4 sounds like left tp and right internal, no idea
  // 0 TP_LEFT, 1 TP_RIGHT, 3 INT_LEFT, 4 INT_RIGHT*/

  uint8_t channels[] = {2, 5};

  for (uint8_t ch : channels) {
    MsgHapticPCMMode packetDisable;
    packetDisable.operation = static_cast<uint8_t>(TritonPCMOperation::DISABLE);
    packetDisable.side = ch;
    packetDisable.param = 0;

    sendPCMMode(&packetDisable);
  }
  if (mode == TritonPCMMode::None) return;
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  for (uint8_t ch : channels) {
    MsgHapticPCMMode packet;
    packet.operation = static_cast<uint8_t>(TritonPCMOperation::ENABLE);
    packet.side = ch;
    packet.param = static_cast<uint8_t>(mode);
    sendPCMMode(&packet);
  }
}

// can be called even if already running to change if updates should be processed (and if they are, make sure you are occasionally using pollUpdates, or updates will end up just leaking memory)
void TritonController::startPoll(bool processUpdate) {
  processUpdates.store(processUpdate);
  if (pollThread.joinable()) return;
  running.store(true);
  pollThread = std::thread([this]() {
    pollLoop();
  });
}

void TritonController::stopPoll() {
  running.store(false);
  processUpdates.store(false);
}

std::vector<TritonInputUpdate> TritonController::pollUpdates() {
  std::scoped_lock lock(inputMutex);
  std::vector<TritonInputUpdate> updates = pendingInputUpdates;
  pendingInputUpdates.clear();
  return updates;
}

void TritonController::pollLoop() {
  while (running.load()) {
    uint8_t buff[64] = {0};
    int read = readRaw(buff, 64);
    if (read == 0 || read == -1) continue;
    uint8_t reportID = buff[0];

    // remove report id;
    std::memmove(buff, buff + 1, 63);

    switch (static_cast<ETritonReportIDTypes>(reportID)) {
      case ID_TRITON_CONTROLLER_STATE:
        {
          TritonMTUFull_t packet{};
          memcpy(&packet, buff, 53);
          {
            std::scoped_lock lock(inputMutex);
            if (processUpdates.load()) {
              TritonInputUpdate updt{};
              updt.state = packet;
              updt.pressed = packet.buttons & ~_state.buttons;
              updt.released = _state.buttons & ~packet.buttons;
              pendingInputUpdates.push_back(updt);
            }
          }
          std::scoped_lock lock(stateMutex);
          this->_state = packet;
          stateCounter++;
          break;
        }
      case ID_TRITON_BATTERY_STATUS:
        {
          TritonBatteryStatus_t battPacket{};
          memcpy(&battPacket, buff, 14);
          std::scoped_lock lock(batteryMutex);
          this->_battery = battPacket;
          batteryCounter++;
          break;
        }
      case ID_TRITON_CONTROLLER_STATE_BLE:
      case ID_TRITON_CONTROLLER_STATE_TIMESTAMP:
      case ID_TRITON_WIRELESS_STATUS:
      case ID_TRITON_WIRELESS_STATUS_X:
      default:
        break;
    }
  }
}
