#include "portaudio.h"
#include <cstdlib>
#include <iostream>
#include <cmath>

// callback used with portAudio
static int TestCallback(  const void* inputBuffer,
                          void* outputBuffer,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData  );

// prints some basic error info
static void PrintErrInfo(PaError err);

int main(int argc, char** argv) {
  PaError err = Pa_Initialize();
  std::cout << err << std::endl;
  std::cout << paNoError << std::endl;
  if (err != paNoError) {
    // error occurred
    PrintErrInfo(err);
    return EXIT_FAILURE;
  }

  std::cout << "oh yeah kill em" << std::endl;
  err = Pa_Terminate();
  if (err != paNoError) {
    PrintErrInfo(err);
  }
  return EXIT_SUCCESS;
}

static void PrintErrInfo(PaError err) {
  std::cout << "An error occurred: " << Pa_GetErrorText(err) << std::endl;
}

// generate some funny noise
static int TestCallback(  const void* inputBuffer,
                          void* outputBuffer,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData  ) 
{
  // input buffer can be ignored
  // sample uses userdata to store gen info
  // in the real deal we can use userData to store a struct which
  // contains, among other things, the critical buffer.
  return 0;
}