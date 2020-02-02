#include <iostream>

#include "portaudio.h"
#include "pawrapper.hpp"

PaWrapper* PaWrapper::tracker = 0;

PaWrapper* PaWrapper::GetWrapper() {
  if (!tracker) {
    tracker = new PaWrapper;  // note: must be freed afterwards
  }

  return tracker;
}

bool PaWrapper::Initialize() {
  PaError err = Pa_Initialize();
  if (err) {
    std::cerr << "An error has occured: " << Pa_GetErrorText(err)
      << std::endl;
    return false;
  }

  // ensure devices are initialized
  device_count = Pa_GetDeviceCount();

  if (device_count == 0) {
    std::cerr << "No devices initialized!" << std::endl;
  } else if (device_count < 0) {
    std::cerr << "clearly something has gone wrong: " << Pa_GetErrorText(device_count)
      << std::endl;
  } else {
    std::cout << "Initialized " << device_count << " devices."
      << std::endl;
  }
 
  return true;
}

void PaWrapper::GetDevices() {
  
}

PaWrapper::~PaWrapper() {
  // nothin yet :)
}
