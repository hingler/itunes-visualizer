#include "portaudio.h"
#include <cstdlib>
#include <iostream>
#include <cmath>

struct Period {
  float left_phase;
  float right_phase;
};

void RadWrap(float&);

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
  if (err != paNoError) {
    // error occurred
    PrintErrInfo(err);
    return EXIT_FAILURE;
  }

  // at this point our stream is up
  std::cout << "oh yeah kill em" << std::endl;

  // check device list
  int numDevices = Pa_GetDeviceCount();
  if (numDevices <= 0) {
    std::cout << "no devices available." << std::endl;
    std::cout << numDevices << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "devices available: " << numDevices << std::endl;

  PaStream* stream;   // file-like IO
  Period data = {0.0f, 0.0f};
  err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, 48000, paFramesPerBufferUnspecified, TestCallback, &data);
  if (!err == paNoError) {
    PrintErrInfo(err);
    return EXIT_FAILURE;
  }
  // start playback
  err = Pa_StartStream(stream);
  if (err != paNoError) {
    PrintErrInfo(err);
    return EXIT_FAILURE;
  }
  // let playback continue
  Pa_Sleep(2000);
  // end the stream
  err = Pa_StopStream(stream);
  if (!err == paNoError) {
    PrintErrInfo(err);
    return EXIT_FAILURE;
  }
  // terminate PA
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

  // data is interlaced, seemingly, in pairs [left, right]

  // userdata: our phase tracker
  Period* p = reinterpret_cast<Period*>(userData);
  float* output = reinterpret_cast<float*>(outputBuffer);

  // frame: single sample across all speakers
  for (int i = 0; i < frameCount; i++) {
    *output++ = sin(p->left_phase) * 0.25f;
    *output++ = sin(p->right_phase) * 0.25f;
    p->left_phase += .06f;
    p->right_phase += .065f;
    RadWrap(p->left_phase);
    RadWrap(p->right_phase);
  }

  return 0;
}

void RadWrap(float& in) {
  if (in >= M_2_PI) {
    in -= M_2_PI;
  }
}