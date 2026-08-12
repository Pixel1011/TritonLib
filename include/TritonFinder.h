#pragma once
#include "TritonController.h"
#include <cstdint>
#include <iostream>

class TritonFinder {
private:
  bool silent = false;
  hid_device* open_steam_controller_hid(uint16_t pid);

public:
  TritonFinder(bool silent = false);
  ~TritonFinder();

  TritonController* getController();
};