// stubcode for portaudio
// used for testing because verifying opaque PA calls is a pain in the ass

#define paNoError 0
#define paFloat32 1
#define paFramesPerBufferUnspecified 6000000
#define paNoFlag 3

// for assertions
#include "gtest/gtest.h"

#include <thread>
#include <atomic>

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

// TESTING BITS

static bool is_pa_active = false;

// represents the callback thread
static std::thread running_thread;

// references a heap allocated vorbis in memory, used to ensure our callback function returns correctly
static float* audio_comparison;

// INFRASTRUCTURE BITS

// 32 bytes
// packet created by application
// just make sure the values are semi valid
struct PaStreamParameters {
  double suggestedLatency;
  void* hostApiSpecificStreamInfo;
  long sampleFormat;
  int channelCount;
  int device;
};

// used to represent streams and deviceinfo
struct PaDeviceInfo {
  double defaultLowOutputLatency;
  double defaultHighOutputLatency;
  double defaultSampleRate;
  char* name;
  int maxOutputChannels;
};

struct PaStream {
  PaCallback* callback;
  void* userdata;
  unsigned long framecount;
  std::thread callback_thread;
  std::atomic_flag callback_signal;
}

// unused atm
struct PaStreamCallbackFlags {
  char cum;
};

// unused
struct PaStreamCallbackTimeInfo {
  char cumtwo;
};

// callback shit
typedef int(*PaCallback)(   const void* input,
                            void* output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo* info,
                            PaStreamCallbackFlags statusFlags,
                            void* userdata  );

int Pa_Initialize() {
  // read sample file
  // allocate space
  // store in static float
  // pa_active = true
}

int Pa_Terminate() {
  // free static float space
  // pa_active = false
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

  // heap allocate a pa stream
  // set it up
  // return a non error value
  return paNoError;
}

// nothing really
char* Pa_GetErrorText(int err) {
  char* hello = new char[16];
  return hello;
}

// dummy result
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

// actually sleeping is probably best
void Pa_Sleep(int time) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

// dummy value
int Pa_GetDeviceCount() {
  return 16;
}




