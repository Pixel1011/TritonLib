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
  hid_device* open_steam_controller_hid(uint16_t pid);
  public:
  TritonFinder();
  ~TritonFinder();

  TritonController* getController();
  
};