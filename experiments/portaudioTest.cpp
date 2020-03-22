#include "portaudio.h"
#include "vorbis/stb_vorbis.h"
#include <cstdlib>
#include <iostream>
#include <cmath>

struct Period {
  float left_phase;
  float right_phase;
  stb_vorbis* stream;
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

  const PaDeviceInfo* deviceInfo;

  for (int i = 0; i < numDevices; i++) {
    deviceInfo = Pa_GetDeviceInfo(i);
    std::cout << "device " << i << ": " << deviceInfo->name << std::endl;
    std::cout << deviceInfo->defaultSampleRate << "Hz" << std::endl;
  }

  deviceInfo = Pa_GetDeviceInfo(1); // try it

  PaStreamParameters outParams;
  outParams.channelCount = deviceInfo->maxOutputChannels;

  // note: this is specific to my machine and probably wont work
  // we are just messing arond :)
  outParams.device = 1; // well thats what it is
  outParams.hostApiSpecificStreamInfo = NULL;
  outParams.sampleFormat = paFloat32;
  outParams.suggestedLatency = deviceInfo->defaultLowOutputLatency;

  std::cout << "devices available: " << numDevices << std::endl;

  PaStream* stream;   // file-like IO

  // set up the vorbis
  int errv;
  stb_vorbis* vorb = stb_vorbis_open_filename("../experiments/flap_jack_scream.ogg", &errv, NULL);



  Period data = {0.0f, 0.0f, vorb};
  err = Pa_OpenStream(&stream, NULL, &outParams, 44100, paFramesPerBufferUnspecified, paNoFlag, TestCallback, &data);
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
  Pa_Sleep(2500);
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
  stb_vorbis_get_samples_float_interleaved(p->stream, 2, output, frameCount * 2);
  return 0;
}

void RadWrap(float& in) {
  if (in >= M_PI * 2) {
    in -= M_PI * 2;
  }
}