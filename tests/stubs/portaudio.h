// stubcode for portaudio

#include <thread>

#define paNoError 0
#define paFloat32 1
#define paFramesPerBufferUnspecified 2
#define paNoFlag 3

// not important

typedef int paError;

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
};

typedef PaStream PaDeviceInfo;

struct PaStreamCallbackFlags {
  char cum;
};



typedef PaStreamCallbackFlags PaStreamCallbackTimeInfo;


typedef void(*PaCallback)(  void* input,
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
}





