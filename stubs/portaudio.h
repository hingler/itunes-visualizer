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

static bool is_pa_active = false;

// represents the callback thread
static std::thread running_thread;
static char* file_ref;

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
  PaCallback callback;
  void* userdata;
  unsigned long framecount;
  std::thread callback_thread;
  std::atomic_flag callback_signal;
  int channel_count;
};

void Set_Filename(char* name) {
  file_ref = name;
}


int Pa_Initialize() {
  int err;
  stb_vorbis* stream = stb_vorbis_open_filename(file_ref, &err, NULL);
  stb_vorbis_seek_start(stream);
  audio_comparison = new float[stb_vorbis_stream_length_in_samples(stream)];
   int bytes_read;
   int offset = 0;
   do {
     bytes_read = stb_vorbis_get_samples_float_interleaved(stream, 2, &audio_comparison[offset], 1024);
     offset += bytes_read;
   } while (bytes_read != 0);
  // read sample file
  // allocate space
  // store in static float

  // i mean this is just shit
  #ifdef GTEST_API_
    ASSERT_FALSE(is_pa_active);
  #endif
  is_pa_active = true;
  return paNoError;
}

int Pa_Terminate() {
  // free static float space
  delete[] audio_comparison;
  // pa_active = false
  return paNoError;
}

static std::thread thread;

int Pa_OpenStream(PaStream** stream,
                  PaStreamParameters* inparam,
                  PaStreamParameters* outparam,
                  double sample_rate,
                  long framecount,
                  int flags,
                  PaCallback callback,
                  void* userdata  )
{
  *stream = new PaStream();
  (*stream)->callback = callback;
  (*stream)->framecount = framecount;
  (*stream)->userdata = userdata;
  (*stream)->channel_count = outparam->channelCount;
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
  ret->maxOutputChannels = 2;
  return ret;
}

int Pa_StartStream(PaStream* stream) {
  // start the thread here
  stream->callback_thread = std::thread(Dummy_ThreadFunc, stream);
  return paNoError;
}

int Pa_StopStream(PaStream* stream) {
  // send kill signal to thread
  // join it
  stream->callback_signal.clear();
  stream->callback_thread.join();
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

void Dummy_ThreadFunc(PaStream* stream) {
  // code which runs the thread, until it is done!
  // it is done when the signal is sent by stopstream
  float max_output[2048];
  int offset = 0;
  int framecount = stream->framecount;
  while (stream->callback_signal.test_and_set()) {
    if (stream->framecount == paFramesPerBufferUnspecified) {
      switch (rand() % 3) {
        case 0:
          framecount = 256;
          break;
        case 1:
          framecount = 512;
          break;
        case 2:
          framecount = 1024;
          break;
        default:
          framecount = 128;
      }
    }

    // todo: handle returned args
    stream->callback(nullptr, max_output, framecount, nullptr, {'c'}, stream->userdata);
    for (int i = 0; i < framecount * stream->channel_count; i++) {
      // verify against audio reference
      #ifdef GTEST_API_
        ASSERT_EQ(max_output[i], audio_comparison[offset + i]);
      #endif
    }

    offset += (framecount * stream->channel_count);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

#endif  // PORTAUDIO_H_