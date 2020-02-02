// worldwide infanticide
#ifndef PAWRAPPER_H_
#define PAWRAPPER_H_
#include "portaudio.h"

class PaWrapper {
 private:
  static PaWrapper* tracker;  // singleton

  PaDeviceIndex device_count;  // number of devices, as det'd by PA

  PaWrapper();
  PaWrapper(const PaWrapper&) = delete;
  PaWrapper& operator=(const PaWrapper&) = delete;

 public:

  /**
   * Returns the currently active wrapper, or constructs a new one.
   */ 
  static PaWrapper* GetWrapper();

  /**
   * Attempts to initialize PA. True on success, false on error
   * plus printed to console.
   */ 
  bool Initialize();

  // debug shit
  void GetDevices();
  
  ~PaWrapper();

};  // class PaWrapper;

#endif  // PAWRAPPER_H_
