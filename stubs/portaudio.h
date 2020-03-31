// stubcode for portaudio
// used for testing because verifying opaque PA calls is a pain in the ass

#define paNoError 0
#define paFloat32 1
#define paFramesPerBufferUnspecified 2
#define paNoFlag 3

// for assertions
#include "gtest/gtest.h"

// TODO: Flesh out the stub code to do some
//       verification of inputs/outputs

//        Additionally: the threading code should be decent
//        to ensure that the input/output functions correctly
//        it may be necessary to use some setup functions
//        which the actual test code can hook into in order to
//        pass audio content

#include <thread>
#include <chrono>

typedef int PaError;

// pacallback: read the buffer and just check it
// initialize/terminate: return paNoError

// getdeviceinfo: return the same thing every time (some dummy format)
// most pointers will just map to void* unless params are necessary

// 32 bytes
struct PaStreamParameters {
  double suggestedLatency;
  void* hostApiSpecificStreamInfo;
  long sampleFormat;
  int channelCount;
  int device;
};

struct PaStream {
  double defaultLowOutputLatency;
  double defaultHighOutputLatency;
  int maxOutputChannels;
  bool isStream;
  char* name;
  double defaultSampleRate;
};

typedef PaStream PaDeviceInfo;

struct PaStreamCallbackFlags {
  char cum;
};

struct PaStreamCallbackTimeInfo {
  char cumtwo;
};

typedef int(*PaCallback)(   const void* input,
                            void* output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo* info,
                            PaStreamCallbackFlags statusFlags,
                            void* userdata  );

int Pa_Initialize() {
  return paNoError;
}

int Pa_Terminate() {
  return paNoError;
}

static std::thread thread;

int Pa_OpenStream(PaStream**,
                  PaStreamParameters* inparam,
                  PaStreamParameters* outparam,
                  double sample_rate,
                  long framecount,
                  int flags,
                  PaCallback callback,
                  void* userdata  )
{

  // setup a thread which will call our callback
  return 1;
}

char* Pa_GetErrorText(int err) {
  char* hello = new char[16];
  return hello;
}

const PaDeviceInfo* Pa_GetDeviceInfo(int num) {
  PaDeviceInfo* ret = new PaDeviceInfo();
  // memory leak but who gives a shit
  ret->defaultHighOutputLatency = 100;
  ret->defaultLowOutputLatency = 50;
  ret->isStream = false;
  ret->maxOutputChannels = 2;
  return ret;
}

int Pa_StartStream(PaStream* stream) {
  // start the thread here
  return paNoError;
}

int Pa_StopStream(PaStream* stream) {
  return paNoError;
}

int Pa_CloseStream(PaStream* stream) {
  return paNoError;
}

void Pa_Sleep(int time) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

int Pa_GetDeviceCount() {
  return 16;
}




