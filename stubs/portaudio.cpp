#include "portaudio.h"

#include <iostream>

// oops, these need to be in a linked cpp file instead :)
bool is_pa_active = false;

// represents the callback thread
std::thread running_thread;

std::string* file_ref;

// references a heap allocated vorbis in memory, used to ensure our callback function returns correctly
float* audio_comparison;

void Set_Filename(std::string name) {
  file_ref = new std::string();
  *file_ref = name;
}

int Pa_Initialize() {
  int err;
  stb_vorbis* stream = stb_vorbis_open_filename(file_ref->c_str(), &err, NULL);
  if (stream == NULL) {
    std::cout << file_ref << std::endl;
  }

  stb_vorbis_seek_start(stream);
  
  int bytes_left = stb_vorbis_stream_length_in_samples(stream) * 2;
  std::cout << "length: " << bytes_left << std::endl;
  // audio_comparison = new float[bytes_left];
  // int bytes_read;
  // int offset = 0;
  // do {
  //   bytes_read = stb_vorbis_get_samples_float_interleaved(stream, 2, &audio_comparison[offset], (bytes_left > 1024 ? 1024 : bytes_left));
  //   offset += bytes_read;
  //   // oops
  //   bytes_left -= bytes_read;
  // } while (bytes_read != 0);
  // read sample file
  // allocate space
  // store in static float

  // i mean this is just shit
  #ifdef GTEST_API_
    ASSERT_FALSE(is_pa_active);
  #endif
  is_pa_active = true;
  std::cout << "all good" << std::endl;
  stb_vorbis_close(stream);
  return paNoError;
}

int Pa_Terminate() {
  // free static float space
  // delete[] audio_comparison;
  is_pa_active = false;
  return paNoError;
}

std::thread thread;

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
  (*stream)->sample_rate = sample_rate;
  (*stream)->channel_count = outparam->channelCount;
  // heap allocate a pa stream
  // set it up
  // return a non error value
  return paNoError;
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
  ret->maxOutputChannels = 2;
  return ret;
}

void Dummy_ThreadFunc(PaStream* stream) {
  std::cout << "run once" << std::endl;
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

    // std::cout << "framecount" << framecount << std::endl;

    // todo: handle returned args
    // dummy val passed to callback
    stream->callback(nullptr, max_output, framecount, nullptr, {'c'}, stream->userdata);
    // std::cout << "ran callback!" << std::endl;

    offset += (framecount * stream->channel_count);
    // std::cout << "new offset: " << offset << std::endl;

    // number of samples processed
    int sleep_time = 980000000 * (framecount / stream->sample_rate);
    // number of nanoseconds of audio

    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
  }
}

int Pa_StartStream(PaStream* stream) {
  // start the thread here
  std::cout << "starting the stream thread!" << std::endl;
  stream->callback_signal.test_and_set();
  stream->callback_thread = std::thread(Dummy_ThreadFunc, stream);
  std::cout << "all good" << std::endl;
  return paNoError;
}

int Pa_StopStream(PaStream* stream) {
  // send kill signal to thread
  // join it
  stream->callback_signal.clear();
  stream->callback_thread.join();
  delete stream;
  return paNoError;
}

int Pa_CloseStream(PaStream* stream) {
  Pa_StopStream(stream);
  return paNoError;
}

void Pa_Sleep(int time) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

int Pa_GetDeviceCount() {
  return 16;
}