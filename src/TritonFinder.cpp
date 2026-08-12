#include "TritonFinder.h"

hid_device* TritonFinder::open_steam_controller_hid(uint16_t pid) {
  struct hid_device_info* devs = hid_enumerate(0x28DE, pid);
  hid_device* handle = nullptr;
  uint8_t buf[64];
  for (struct hid_device_info* cur = devs; cur != nullptr; cur = cur->next) {
    if (cur->usage_page != 0xFF00) continue;
    handle = hid_open_path(cur->path);

    if (handle) {
      int r = hid_read_timeout(handle, buf, sizeof(buf), 100);
      if (r > 0) {
        hid_free_enumeration(devs);
        return handle;
      }
      hid_close(handle);
    }

    handle = nullptr;
  }
  hid_free_enumeration(devs);
  return handle;
}

TritonFinder::TritonFinder(bool silent) {
  // init hidAPI
  this->silent = silent;
  int r = hid_init();
  if (r != 0) {
    std::cerr << "HIDAPI Init Error: " << r << std::endl;
    std::cin.ignore();
    throw "HIDAPI Init Error";
  }
}

TritonFinder::~TritonFinder() {
  hid_exit();
}

// basically completely stolen from SteamHapticsSinger
TritonController* TritonFinder::getController() {
  ETritonConnectionType type = ETritonConnectionType::k_ETritonConnectionType_Unknown;

  hid_device* hid_handle;

  // could also be ble but bleh
  ETritonPairType pairType = ETritonPairType::k_ETritonPairType_None;
  // Open Steam Controller device

  if ((hid_handle = this->open_steam_controller_hid(0x1302)) != nullptr) { // Steam Controller (2026)
    if (!silent) std::cout << "Found wired Steam Controller (2026)" << std::endl;
    type = ETritonConnectionType::k_ETritonConnectionType_USB;

  } else if ((hid_handle = this->open_steam_controller_hid(0x1304)) != nullptr) { // Steam Puck
    if (!silent) std::cout << "Found Steam Puck, using first Steam Controller (2026)" << std::endl;
    type = ETritonConnectionType::k_ETritonConnectionType_Puck;

  } else if ((hid_handle = this->open_steam_controller_hid(0x1305)) != nullptr) { // Steam Machine internal puck
    if (!silent) std::cout << "Found Steam Machine internal puck, using first Steam Controller (2026)" << std::endl;
    type = ETritonConnectionType::k_ETritonConnectionType_Machine;
  } else {
    if (!silent) std::cout << "No device found" << std::endl;
  }

  switch (type) {
    case ETritonConnectionType::k_ETritonConnectionType_Unknown:
      return nullptr;
      break;

    case ETritonConnectionType::k_ETritonConnectionType_USB:
    case ETritonConnectionType::k_ETritonConnectionType_Puck:
    case ETritonConnectionType::k_ETritonConnectionType_Machine:
      return new TritonController(hid_handle, pairType);
      break;
    default:
      return nullptr;
      break;
  }
};