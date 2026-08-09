#pragma once
#include "TritonController.h"
#include <iostream>
#include <cstdint>

enum class ControllerType {
	None,
	Original,
	Triton,
	//Jupiter,
	//Galileo
};

class TritonFinder {
  private:
  bool silent = false;
  hid_device* open_steam_controller_hid(uint16_t pid);
  public:
  TritonFinder(bool silent = false);
  ~TritonFinder();

  TritonController* getController();
  
};