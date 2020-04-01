// stubcode for portaudio
// used for testing because verifying opaque PA calls is a pain in the ass

#ifndef PORTAUDIO_H_
#define PORTAUDIO_H_

#define paNoError 0
#define paFloat32 1
#define paFramesPerBufferUnspecified 6000000
#define paNoFlag 3

// for assertions
#include "vorbis/stb_vorbis.h"

#include <thread>
#include <atomic>
#include <chrono>


typedef int PaError;

// pacallback: read the buffer and just check it
// initialize/terminate: return paNoError

// getdeviceinfo: return the same thing every time (some dummy format)
// most pointers will just map to void* unless params are necessary

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

// TESTING BITS

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
  PaCallback callback;
  void* userdata;
  unsigned long framecount;
  std::thread callback_thread;
  std::atomic_flag callback_signal;
  double sample_rate;
  int channel_count;
};


void Set_Filename(std::string name);

int Pa_Initialize();

int Pa_Terminate();

int Pa_OpenStream(PaStream** stream,
                  PaStreamParameters* inparam,
                  PaStreamParameters* outparam,
                  double sample_rate,
                  long framecount,
                  int flags,
                  PaCallback callback,
                  void* userdata  );

// nothing really
char* Pa_GetErrorText(int err);

// dummy result
const PaDeviceInfo* Pa_GetDeviceInfo(int num);

void Dummy_ThreadFunc(PaStream* stream);

int Pa_StartStream(PaStream* stream);

int Pa_StopStream(PaStream* stream);

int Pa_CloseStream(PaStream* stream);

// actually sleeping is probably best
void Pa_Sleep(int time);

// dummy value
int Pa_GetDeviceCount();

#endif  // PORTAUDIO_H_