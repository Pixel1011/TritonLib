#pragma once
#include <mutex>
#include <thread>
#include <atomic>
#include <hidapi.h>
#include <cstdint>
#include <functional>
#define HID_FEATURE_REPORT_BYTES 64
// have not tested 0x87 and 0x89. i could be completely wrong about them


// note to fix rumble as it will timeout after 50ms, so like sdl, resend every 40, but need to do funny thread stuff with poll loop
// also when using lizard mode off, should have a var that updates it every 3 secs to avoid it going back to on

#pragma region structs
#pragma pack(push, 1)

/*Structs taken directly from SDL code*/
// ill just add this to not be shot
/*
  Simple DirectMedia Layer
  Copyright (C) 2020 Valve Corporation
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

// feature report commands
// Header for all host <==> target messages
typedef struct
{
  unsigned char type;
  unsigned char length;
} FeatureReportHeader;

// Generic controller settings structure
typedef struct
{
  unsigned char settingNum;
  unsigned short settingValue;
} ControllerSetting;

// Generic controller attribute structure
typedef struct
{
  unsigned char attributeTag;
  uint32_t attributeValue;
} ControllerAttribute;

// Generic controller settings structure
typedef struct
{
  ControllerSetting settings[(HID_FEATURE_REPORT_BYTES - sizeof(FeatureReportHeader)) / sizeof(ControllerSetting)];
} MsgSetSettingsValues, MsgGetSettingsValues, MsgGetSettingsDefaults, MsgGetSettingsMaxs;

// Generic controller settings structure
typedef struct
{
  ControllerAttribute attributes[(HID_FEATURE_REPORT_BYTES - sizeof(FeatureReportHeader)) / sizeof(ControllerAttribute)];
} MsgGetAttributes;

typedef struct
{
  unsigned char attributeTag;
  char attributeValue[20];
} MsgGetStringAttribute;

typedef struct
{
  unsigned char mode;
} MsgSetControllerMode;

typedef struct
{
  FeatureReportHeader header;
  union {
    MsgSetSettingsValues setSettingsValues;
    MsgGetSettingsValues getSettingsValues;
    MsgGetSettingsMaxs getSettingsMaxs;
    MsgGetSettingsDefaults getSettingsDefaults;
    MsgGetAttributes getAttributes;
    MsgSetControllerMode controllerMode;
    MsgGetStringAttribute getStringAttribute;
  } payload;

} FeatureReportMsg;

// i wish we had all of the enums instead of just lizard mode :(
// and figuring it all out manually will take a ridiculous amount of time (i am very, very happy with pull requests if anyone wishes to help :D)

typedef enum
{
	LIZARD_MODE_OFF,
	LIZARD_MODE_ON,
} LizardModeState_t;

// Read-write controller settings (only add to this enum and never change the order) I will leave this as is with the defs from SDL, though will add comments for each once i properly learn
typedef enum 
{
	SETTING_MOUSE_SENSITIVITY,
	SETTING_MOUSE_ACCELERATION,
	SETTING_TRACKBALL_ROTATION_ANGLE,
	SETTING_HAPTIC_INTENSITY_UNUSED,
  
	SETTING_LEFT_GAMEPAD_STICK_ENABLED,
	SETTING_RIGHT_GAMEPAD_STICK_ENABLED,
	SETTING_USB_DEBUG_MODE,
	SETTING_LEFT_TRACKPAD_MODE,
	SETTING_RIGHT_TRACKPAD_MODE,
  // steam sends this one (as 01 87 03 09 00 00) every 3 secs or so.
  // does what it says, use LizardModeState_t
	SETTING_LIZARD_MODE, 

	// 10
	SETTING_DPAD_DEADZONE,
	SETTING_MINIMUM_MOMENTUM_VEL,
	SETTING_MOMENTUM_DECAY_AMOUNT,
	SETTING_TRACKPAD_RELATIVE_MODE_TICKS_PER_PIXEL,
	SETTING_HAPTIC_INCREMENT,
	SETTING_DPAD_ANGLE_SIN,
	SETTING_DPAD_ANGLE_COS,
	SETTING_MOMENTUM_VERTICAL_DIVISOR,
	SETTING_MOMENTUM_MAXIMUM_VELOCITY,
	SETTING_TRACKPAD_Z_ON,

	// 20
	SETTING_TRACKPAD_Z_OFF,
	SETTING_SENSITIVITY_SCALE_AMOUNT,
	SETTING_LEFT_TRACKPAD_SECONDARY_MODE,
	SETTING_RIGHT_TRACKPAD_SECONDARY_MODE,
	SETTING_SMOOTH_ABSOLUTE_MOUSE,
	SETTING_STEAMBUTTON_POWEROFF_TIME,
	SETTING_UNUSED_1,
	SETTING_TRACKPAD_OUTER_RADIUS,
	SETTING_TRACKPAD_Z_ON_LEFT,
	SETTING_TRACKPAD_Z_OFF_LEFT,

	// 30
	SETTING_TRACKPAD_OUTER_SPIN_VEL,
	SETTING_TRACKPAD_OUTER_SPIN_RADIUS,
	SETTING_TRACKPAD_OUTER_SPIN_HORIZONTAL_ONLY,
	SETTING_TRACKPAD_RELATIVE_MODE_DEADZONE,
	SETTING_TRACKPAD_RELATIVE_MODE_MAX_VEL,
	SETTING_TRACKPAD_RELATIVE_MODE_INVERT_Y,
	SETTING_TRACKPAD_DOUBLE_TAP_BEEP_ENABLED,
	SETTING_TRACKPAD_DOUBLE_TAP_BEEP_PERIOD,
	SETTING_TRACKPAD_DOUBLE_TAP_BEEP_COUNT,
	SETTING_TRACKPAD_OUTER_RADIUS_RELEASE_ON_TRANSITION,

	// 40
	SETTING_RADIAL_MODE_ANGLE,
	SETTING_HAPTIC_INTENSITY_MOUSE_MODE,
	SETTING_LEFT_DPAD_REQUIRES_CLICK,
	SETTING_RIGHT_DPAD_REQUIRES_CLICK,
	SETTING_LED_BASELINE_BRIGHTNESS,
	SETTING_LED_USER_BRIGHTNESS,
	SETTING_ENABLE_RAW_JOYSTICK,
	SETTING_ENABLE_FAST_SCAN,
	SETTING_IMU_MODE,
	SETTING_WIRELESS_PACKET_VERSION,

	// 50
	SETTING_SLEEP_INACTIVITY_TIMEOUT,
	SETTING_TRACKPAD_NOISE_THRESHOLD,
	SETTING_LEFT_TRACKPAD_CLICK_PRESSURE,
	SETTING_RIGHT_TRACKPAD_CLICK_PRESSURE,
	SETTING_LEFT_BUMPER_CLICK_PRESSURE,
	SETTING_RIGHT_BUMPER_CLICK_PRESSURE,
	SETTING_LEFT_GRIP_CLICK_PRESSURE,
	SETTING_RIGHT_GRIP_CLICK_PRESSURE,
	SETTING_LEFT_GRIP2_CLICK_PRESSURE,
	SETTING_RIGHT_GRIP2_CLICK_PRESSURE,

	// 60
	SETTING_PRESSURE_MODE,
	SETTING_CONTROLLER_TEST_MODE,
	SETTING_TRIGGER_MODE,
	SETTING_TRACKPAD_Z_THRESHOLD,
	SETTING_FRAME_RATE,
	SETTING_TRACKPAD_FILT_CTRL,
	SETTING_TRACKPAD_CLIP,
	SETTING_DEBUG_OUTPUT_SELECT,
	SETTING_TRIGGER_THRESHOLD_PERCENT,
	SETTING_TRACKPAD_FREQUENCY_HOPPING,

	// 70
	SETTING_HAPTICS_ENABLED,
	SETTING_STEAM_WATCHDOG_ENABLE,
	SETTING_TIMP_TOUCH_THRESHOLD_ON,
	SETTING_TIMP_TOUCH_THRESHOLD_OFF,
	SETTING_FREQ_HOPPING,
	SETTING_TEST_CONTROL,
	SETTING_HAPTIC_MASTER_GAIN_DB,
	SETTING_THUMB_TOUCH_THRESH,
	SETTING_DEVICE_POWER_STATUS,
	SETTING_HAPTIC_INTENSITY,

	// 80
	SETTING_STABILIZER_ENABLED,
	SETTING_TIMP_MODE_MTE,
	SETTING_COUNT,
	
	// This is a special setting value use for callbacks and should not be set/get explicitly.
	SETTING_ALL=0xFF
} ControllerSettings;

// probably not all relating to the steam controller, though sure could be alot of them
enum FeatureReportMessageIDs
{
	ID_SET_DIGITAL_MAPPINGS              = 0x80,
	ID_CLEAR_DIGITAL_MAPPINGS            = 0x81,
	ID_GET_DIGITAL_MAPPINGS              = 0x82,
	ID_GET_ATTRIBUTES_VALUES             = 0x83,
	ID_GET_ATTRIBUTE_LABEL               = 0x84,
	ID_SET_DEFAULT_DIGITAL_MAPPINGS      = 0x85,
	ID_FACTORY_RESET                     = 0x86,
	ID_SET_SETTINGS_VALUES               = 0x87,
	ID_CLEAR_SETTINGS_VALUES             = 0x88,
	ID_GET_SETTINGS_VALUES               = 0x89,
	ID_GET_SETTING_LABEL                 = 0x8A,
	ID_GET_SETTINGS_MAXS                 = 0x8B,
	ID_GET_SETTINGS_DEFAULTS             = 0x8C,
	ID_SET_CONTROLLER_MODE               = 0x8D,
	ID_LOAD_DEFAULT_SETTINGS             = 0x8E,
	ID_TRIGGER_HAPTIC_PULSE              = 0x8F,

	ID_TURN_OFF_CONTROLLER               = 0x9F,

	ID_GET_DEVICE_INFO                   = 0xA1,

	ID_CALIBRATE_TRACKPADS               = 0xA7,
	ID_RESERVED_0                        = 0xA8,
	ID_SET_SERIAL_NUMBER                 = 0xA9,
	ID_GET_TRACKPAD_CALIBRATION          = 0xAA,
	ID_GET_TRACKPAD_FACTORY_CALIBRATION  = 0xAB,
	ID_GET_TRACKPAD_RAW_DATA             = 0xAC,
	ID_ENABLE_PAIRING                    = 0xAD,
	ID_GET_STRING_ATTRIBUTE              = 0xAE,
	ID_RADIO_ERASE_RECORDS               = 0xAF,
	ID_RADIO_WRITE_RECORD                = 0xB0,
	ID_SET_DONGLE_SETTING                = 0xB1,
	ID_DONGLE_DISCONNECT_DEVICE          = 0xB2,
	ID_DONGLE_COMMIT_DEVICE              = 0xB3,
	ID_DONGLE_GET_WIRELESS_STATE         = 0xB4,
	ID_CALIBRATE_GYRO                    = 0xB5,
	ID_PLAY_AUDIO                        = 0xB6,
	ID_AUDIO_UPDATE_START                = 0xB7,
	ID_AUDIO_UPDATE_DATA                 = 0xB8,
	ID_AUDIO_UPDATE_COMPLETE             = 0xB9,
	ID_GET_CHIPID                        = 0xBA,

	ID_CALIBRATE_JOYSTICK                = 0xBF,
	ID_CALIBRATE_ANALOG_TRIGGERS         = 0xC0,
	ID_SET_AUDIO_MAPPING                 = 0xC1,
	ID_CHECK_GYRO_FW_LOAD                = 0xC2,
	ID_CALIBRATE_ANALOG                  = 0xC3,
	ID_DONGLE_GET_CONNECTED_SLOTS        = 0xC4,

	ID_RESET_IMU                         = 0xCE,

	// Deck only
	ID_TRIGGER_HAPTIC_CMD                = 0xEA,
	ID_TRIGGER_RUMBLE_CMD                = 0xEB,
};

// hid commands

enum EChargeState {
  k_EChargeStateReset,
  k_EChargeStateDischarging,
  k_EChargeStateCharging,
  k_EChargeStateSrcValidate,
  k_EChargeStateChargingDone,
};

/*
//
  44, upon closing steam - got 14 packets of either
  440102000000 - 10 - 1,3,5,7,9,11,12,13,14
  440002000000 - 4
*/
enum ETritonReportIDTypes {
  ID_TRITON_LIZARD_MOUSE = 0x40,
  ID_TRITON_LIZARD_KEYBOARD = 0x41,
  ID_TRITON_CONTROLLER_STATE = 0x42,
  ID_TRITON_BATTERY_STATUS = 0x43,
  // difference is no quaternion, so use TritonMTUNoQuat_t
  ID_TRITON_CONTROLLER_STATE_BLE = 0x45,
  ID_TRITON_WIRELESS_STATUS_X = 0x46,
  ID_TRITON_CONTROLLER_STATE_TIMESTAMP = 0x47,

  ID_TRITON_WIRELESS_STATUS = 0x79,
  ID_TRITON_QoS_STATUS = 0x7B
};

// packet with data like 7901 / 7902
enum ETritonWirelessState {
  k_ETritonWirelessStateNone = 0,
  k_ETritonWirelessStateDisconnect = 1,
  k_ETritonWirelessStateConnect = 2,
};

typedef enum {

  TRITON_LBUTTON_A = 0x00000001,
  TRITON_LBUTTON_B = 0x00000002,
  TRITON_LBUTTON_X = 0x00000004,
  TRITON_LBUTTON_Y = 0x00000008,

  TRITON_HBUTTON_QAM = 0x00000010,
  TRITON_LBUTTON_R3 = 0x00000020,
  TRITON_LBUTTON_VIEW = 0x00000040,
  TRITON_HBUTTON_R4 = 0x00000080,

  TRITON_LBUTTON_R5 = 0x00000100,
  TRITON_LBUTTON_R = 0x00000200,
  TRITON_LBUTTON_DPAD_DOWN = 0x00000400,
  TRITON_LBUTTON_DPAD_RIGHT = 0x00000800,

  TRITON_LBUTTON_DPAD_LEFT = 0x00001000,
  TRITON_LBUTTON_DPAD_UP = 0x00002000,
  TRITON_LBUTTON_MENU = 0x00004000,
  TRITON_LBUTTON_L3 = 0x00008000,

  TRITON_LBUTTON_STEAM = 0x00010000,
  TRITON_HBUTTON_L4 = 0x00020000,
  TRITON_LBUTTON_L5 = 0x00040000,
  TRITON_LBUTTON_L = 0x00080000,

  TRITON_RIGHT_JOYSTICK_TOUCH = 0x00100000,
  TRITON_RIGHT_TOUCHPAD_TOUCH = 0x00200000,
  TRITON_RIGHT_TOUCHPAD_CLICK = 0x00400000,
  TRITON_RIGHT_TRIGGER_CLICK = 0x00800000,

  TRITON_LEFT_JOYSTICK_TOUCH = 0x01000000,
  TRITON_LEFT_TOUCHPAD_TOUCH = 0x02000000,
  TRITON_LEFT_TOUCHPAD_CLICK = 0x04000000,
  TRITON_LEFT_TRIGGER_CLICK = 0x08000000,

  TRITON_RIGHT_GRIP_TOUCH = 0x10000000,
  TRITON_LEFT_GRIP_TOUCH = 0x20000000,
} TritonButtons;

//output (from host) hid reports
typedef enum {
  RUMBLE = 0x80,
  PULSE = 0x81,
  COMMAND = 0x82,
  LFO_TONE = 0x83,
  LOG_SWEEP = 0x84,
  HAPTIC_SCRIPT = 0x85,
  // guesses, cannot confirm names directly from SDL or otherwise

  PCM_MODE = 0x86,
  PCM_MONO = 0x87,
  PCM_STEREO = 0x88,
  PCM_MONO_WITH_LENGTH = 0x89,
} TritonReportIDs;

typedef struct
{
  uint8_t type;
  uint16_t intensity;
  struct
  {
    uint16_t speed;
    int8_t gain;
  } left, right;
} MsgHapticRumble;

typedef struct
{
  uint8_t side;
  uint16_t on_us;
  uint16_t off_us;
  uint16_t repeat_count;
  uint16_t gain_db;
} MsgHapticPulse;

typedef struct
{
  uint8_t side;
  uint8_t command;
  int8_t gain_db;
} MsgHapticCommand;

typedef struct
{
  uint8_t side;
  int8_t gain_db;
  uint16_t frequency;
  uint16_t duration_ms;
  uint16_t lfo_freq;
  uint8_t lfo_depth;
} MsgHapticLfoTone;

typedef struct
{
  uint8_t side;
  int8_t gain_db;
  uint16_t duration_ms;
  struct
  {
    uint16_t frequency;
  } start, end;
} MsgHapticLogSweep;

typedef struct
{
  uint8_t side;
  uint8_t script_id;
  int8_t gain_db;
} MsgHapticScript;

//14
typedef struct
{
  unsigned char ucChargeState; // EChargeState
  unsigned char ucBatteryLevel;
  unsigned short sBatteryVoltage;
  unsigned short sSystemVoltage;
  unsigned short sInputVoltage;
  unsigned short sCurrent;
  unsigned short sInputCurrent;
  unsigned short sTemperature;
} TritonBatteryStatus_t;

//1
typedef struct
{
  unsigned char state;
} TritonWirelessStatus_t;

//24
typedef struct
{
  uint32_t timestamp;
  short sAccelX;
  short sAccelY;
  short sAccelZ;

  short sGyroX;
  short sGyroY;
  short sGyroZ;

  // While the main output is formatted with these, they are never filled with other than 32767,0,0,0. As the built-in SFLP is set to only output gyro-bias data in firmware rather than also the quaternion data
  short sGyroQuatW;
  short sGyroQuatX;
  short sGyroQuatY;
  short sGyroQuatZ;
} TritonMTUIMU_t;

//16
typedef struct {
  uint32_t timestamp;
  short sAccelX;
  short sAccelY;
  short sAccelZ;

  short sGyroX;
  short sGyroY;
  short sGyroZ;
} TritonMTUIMUNoQuat_t;

//14
typedef struct
{
  uint16_t timestamp;
  short sAccelX;
  short sAccelY;
  short sAccelZ;

  short sGyroX;
  short sGyroY;
  short sGyroZ;
} TritonMTUIMUNoQuat32usTS_t;

//29+24 53
typedef struct
{
  uint8_t seq_num;
  uint32_t buttons;
  short sTriggerLeft;
  short sTriggerRight;

  short sLeftStickX;
  short sLeftStickY;
  short sRightStickX;
  short sRightStickY;

  short sLeftPadX;
  short sLeftPadY;
  unsigned short unPressureLeft;

  short sRightPadX;
  short sRightPadY;
  // 0-32
  unsigned short unPressureRight;
  TritonMTUIMU_t imu;
} TritonMTUFull_t;

//45
typedef struct {
  uint8_t seq_num;
  uint32_t buttons;
  short sTriggerLeft;
  short sTriggerRight;

  short sLeftStickX;
  short sLeftStickY;
  short sRightStickX;
  short sRightStickY;

  short sLeftPadX;
  short sLeftPadY;
  unsigned short unPressureLeft;

  short sRightPadX;
  short sRightPadY;
  unsigned short unPressureRight;
  TritonMTUIMUNoQuat_t imu;
} TritonMTUNoQuat_t;

// New Ibex packet that adds a timestamp to the trackpad sampling
// and reduces the size of the IMU timestamp.  Timestamps are now 16 bits

//45
typedef struct
{
  uint8_t seq_num;
  uint32_t buttons;
  short sTriggerLeft;
  short sTriggerRight;

  short sLeftStickX;
  short sLeftStickY;
  short sRightStickX;
  short sRightStickY;

  unsigned short unTrackpadTimestamp;
  short sLeftPadX;
  short sLeftPadY;
  unsigned short unPressureLeft;

  short sRightPadX;
  short sRightPadY;
  unsigned short unPressureRight;

  TritonMTUIMUNoQuat32usTS_t imu;
} TritonMTUNoQuat32TS_t;

// i believe used for SETTING_IMU_MODE though not sure, was in SDL tho,
// after testing, no idea, 0 does turn it off, but (and i might be dumb because it was only a small amount of testing) anything >0 would just enable everything again and i couldnt get the quaternion to update
typedef enum
{
	SETTING_GYRO_MODE_OFF				= 0x0000,
	SETTING_GYRO_MODE_STEERING			= 0x0001,
	SETTING_GYRO_MODE_TILT				= 0x0002,
	SETTING_GYRO_MODE_SEND_ORIENTATION	= 0x0004,
	SETTING_GYRO_MODE_SEND_RAW_ACCEL	= 0x0008,
	SETTING_GYRO_MODE_SEND_RAW_GYRO		= 0x0010,
} SettingGyroMode;

// Read-write settings for dongle (only add to this enum and never change the order)
// unknown if this one actually is used for anything, though i think
typedef enum 
{
	DONGLE_SETTING_MOUSE_KEYBOARD_ENABLED,
	DONGLE_SETTING_COUNT,
} DongleSettings;

// could be something but absolutely no idea
typedef enum
{
	AUDIO_STARTUP		= 0,
	AUDIO_SHUTDOWN		= 1,
	AUDIO_PAIR			= 2,
	AUDIO_PAIR_SUCCESS	= 3,
	AUDIO_IDENTIFY		= 4,
	AUDIO_LIZARDMODE	= 5,
	AUDIO_NORMALMODE	= 6,

	AUDIO_MAX_SLOT      = 15
} ControllerAudio;

// end of sdl stuff


/*Structs found via RE*/
enum class TritonPCMMode {
  Khz8_16Bit,
  Khz4_16Bit,
  Khz2_16Bit,
  Khz1_16Bit,
  
  Khz8_8Bit,
  Khz4_8Bit,
  Khz2_8Bit,
  Khz1_8Bit,
  
  Khz8_8Bit_ulaw,
  Khz4_8Bit_ulaw,
  Khz2_8Bit_ulaw,
  Khz1_8Bit_ulaw,
  // not a real mode on the controller and cannot be passed in a PCM Mode packet as a param, as you send a 1 on the operation byte to disable if doing it manually. This is purely for setupPCMStreaming.
  None
};

enum class TritonPCMOperation {
  DISABLE = 1,
  ENABLE = 2
};

enum class ETritonPairType {
  k_ETritonPairType_None,
  k_ETritonPairType_Wired,
  k_ETritonPairType_Wireless
};

enum class ETritonConnectionType {
  k_ETritonConnectionType_Unknown,
  k_ETritonConnectionType_Puck,
  k_ETritonConnectionType_Machine,
  k_ETritonConnectionType_USB,
  k_ETritonConnectionType_BT
};

enum class HIDKbModifier : uint8_t {
    MOD_NONE        = 0x00,

    MOD_LEFT_CTRL   = 0x01,
    MOD_LEFT_SHIFT  = 0x02,
    MOD_LEFT_ALT    = 0x04,
    MOD_LEFT_GUI    = 0x08, // Windows key/whatever

    MOD_RIGHT_CTRL  = 0x10,
    MOD_RIGHT_SHIFT = 0x20,
    MOD_RIGHT_ALT   = 0x40,
    MOD_RIGHT_GUI   = 0x80,
};

enum HIDKeyboardKeys
{
	KEY_INVALID,
	KEY_FIRST = 0x04,
	KEY_A = KEY_FIRST, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, 
	KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2, 
	KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_RETURN, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, KEY_SPACE, KEY_DASH, KEY_EQUALS, KEY_LEFT_BRACKET,
	KEY_RIGHT_BRACKET, KEY_BACKSLASH, KEY_UNUSED1, KEY_SEMICOLON, KEY_SINGLE_QUOTE, KEY_BACK_TICK, KEY_COMMA, KEY_PERIOD, KEY_FORWARD_SLASH, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
	KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRINT_SCREEN, KEY_SCROLL_LOCK, KEY_BREAK, KEY_INSERT, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN, KEY_RIGHT_ARROW,
	KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_UP_ARROW, KEY_NUM_LOCK, KEY_KEYPAD_FORWARD_SLASH, KEY_KEYPAD_ASTERISK, KEY_KEYPAD_DASH, KEY_KEYPAD_PLUS, KEY_KEYPAD_ENTER, KEY_KEYPAD_1, KEY_KEYPAD_2, KEY_KEYPAD_3, KEY_KEYPAD_4, KEY_KEYPAD_5, KEY_KEYPAD_6, KEY_KEYPAD_7,
	KEY_KEYPAD_8, KEY_KEYPAD_9, KEY_KEYPAD_0, KEY_KEYPAD_PERIOD,
	KEY_LALT,
  KEY_LSHIFT,
  KEY_LWIN,
  KEY_LCONTROL,
  KEY_RALT,
  KEY_RSHIFT,
  KEY_RWIN,
  KEY_RCONTROL,
	KEY_VOLUP,
	KEY_VOLDOWN,
	KEY_MUTE,
	KEY_PLAY,
	KEY_STOP,
	KEY_NEXT,
	KEY_PREV,
  KEY_LAST = KEY_PREV
};

// Hid mouse buttons
enum MouseButtons
{
	MOUSE_BTN_LEFT,
	MOUSE_BTN_RIGHT,
	MOUSE_BTN_MIDDLE,
	MOUSE_BTN_BACK,
	MOUSE_BTN_FORWARD,
	MOUSE_SCROLL_UP,
	MOUSE_SCROLL_DOWN,
	MOUSE_BTN_COUNT
};

typedef struct
{
  // still dont quite know what any of these 3 do, however these are best guesses
  uint8_t operation;
  uint8_t side;
  uint8_t param; // i believe this is some sort of bitfield or Enum, no idea.
} MsgHapticPCMMode;

typedef struct
{
  uint8_t side;
  char data[62];
} MsgHapticPCMMono;

typedef struct
{
  // <=31
  uint8_t length;

  char left[31];
  char right[31];

} MsgHapticPCMStereo;

typedef struct
{
  uint8_t length;
  uint8_t side;
  char data[61];
} MsgHapticPCMMonoWithLength;

// found from the hell holes of steamclient64.dll and of host_chunk.js protobufs
typedef struct
{
  uint16_t period_ms;
  uint16_t packets_sent;
  uint16_t packet_retransmissions;
  uint16_t interval_max_ms;
  int8_t rssi_measure;
  uint8_t reason;
  uint8_t rf_channel;
  uint8_t backupChannel;
  
} TritonQoSStatus;

typedef struct
{
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t wheel;
  int8_t pan;
} TritonMouseReport;

typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keycodes[6];

} TritonKeyboardReport;

#pragma pack(pop)
#pragma endregion
// my own structs purely for this library, nothing to do with the controller in the hardware sense
struct TritonInputUpdate {
  TritonMTUFull_t state;
  // TritonButtons
  uint32_t pressed;
  // TritonButtons
  uint32_t released;
};

// yes im kinda turning this into a general purpose class for the sc2
// idk im bored
class TritonController {
private:
  hid_device* hid_handle;

  // internal var update stuff
  std::mutex stateMutex;
  std::mutex batteryMutex;
  std::atomic<bool> running = false;
  // updated at 250hz (altho measured 266hz on puck and 248.2hz wired)
  TritonMTUFull_t _state;
  // will get updated once every 3.5 secs
  TritonBatteryStatus_t _battery{};
  std::thread pollThread;
  
  // playing audio stuff
  std::mutex audioMutex;
  std::atomic<bool> playingAudio = false;
  TritonPCMMode currentMode;
  
  // input process stuff
  std::atomic<bool> processUpdates = false;
  std::vector<TritonInputUpdate> pendingInputUpdates;
  std::mutex inputMutex;
  
  void pollLoop();
  int _playStereoAudio(uint8_t pcmBytes[], size_t length, TritonPCMMode mode, std::function<void(int step, int* readPointer)> callback);
  
  public:
  std::thread playThread;

  std::atomic<uint64_t> stateCounter{0};
  std::atomic<uint64_t> batteryCounter{0};
  ETritonPairType pairType{};
  
  // use to check if the controller has disconnected, if true, delete this class and use controller finder to try connect again
  std::atomic<bool> disconnected = false;
  
  TritonController(hid_device* handle, ETritonPairType connection);
  ~TritonController();
  void close();
  int playNote(int channel, int note, int velocity);
  int playFrequency(uint8_t channel, uint16_t frequency, int8_t gaindb, uint16_t durationms = 0xffff, uint16_t lfoFreq = 0, uint8_t lfoDepth = 0);
  // plays audio on the steam controller with given data on a new thread. If called again, will stop and play new audio.
  int playStereoAudio(uint8_t pcmBytes[], size_t length, TritonPCMMode mode, std::function<void(int step, int* readPointer)> callback = [](int step, int* readPointer) {/*do nothing*/});
  int setLizardMode(LizardModeState_t mode);
  
  int sendPCMMode(MsgHapticPCMMode* packet);
  int sendPCMStereo(MsgHapticPCMStereo* packet);
  int sendLFOTone(MsgHapticLfoTone* packet);

  int sendRaw(uint8_t bytes[], size_t length);
  int readRaw(uint8_t buff[], size_t length);
  int sendFeatureReport(FeatureReportMsg* msg, size_t length);

  void setupPCMStreaming(TritonPCMMode mode);
  // reading
  void startPoll(bool processUpdates = false);
  void stopPoll();
  std::vector<TritonInputUpdate> pollUpdates();

  TritonMTUFull_t getFullReport();
  TritonBatteryStatus_t getBatteryStatus();
  bool isPressed(TritonButtons btn);
  // maybe i do pressed/released event queue if bored
};