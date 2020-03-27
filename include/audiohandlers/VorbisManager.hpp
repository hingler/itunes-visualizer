#ifndef VORBIS_MANAGER_H_
#define VORBIS_MANAGER_H_

#include "vorbis/stb_vorbis.h"
#include "audiohandlers/AudioBufferSPSC.hpp"

#include <memory>
#include <mutex>


/**
 *  A read-only wrapper for our AudioBufferSPSC which gives the user single-thread access
 *  to the audio buffer's contents.
 */ 
class ReadOnlyBuffer {
 public:
  /**
   *  All of the following functions wrap the existing functionality for the AudioBufferSPSC class,
   *  with BUFFER_UNIT = float. Please refer to that class for documentation.
   */
  ReadOnlyBuffer(AudioBufferSPSC<float>* buffer) : buffer_(buffer) {}

  size_t Peek(uint32_t count, float** output) {
    return buffer_->Peek(count, output);
  }

  size_t Peek_Chunked(uint32_t framecount, float*** output) {
    return buffer_->Peek_Chunked(framecount, output);
  }

  float* Read(uint32_t count) {
    return buffer_->Read(count);
  }

  float** Read_Chunked(uint32_t framecount) {
    return buffer_->Read_Chunked(framecount);
  }

  void Synchronize(uint32_t sample_num) {
    buffer_->Synchronize(sample_num);
  }

  void Synchronize_Chunked(uint32_t frame_num) {
    buffer_->Synchronize_Chunked(frame_num);
  }

  ~ReadOnlyBuffer() { }

 private:
  std::shared_ptr<AudioBufferSPSC<float>> buffer_;
};

/**
 *  This class encompasses the file I/O associated with playing an audio file via PortAudio.
 */ 
class VorbisManager {
 public:
  /**
   *  Constructs and returns a pointer to a Vorbis Manager from a buffer size and filename.
   *  If the file is invalid, returns nullptr instead.
   * 
   *  Arguments:
   *    - twopow, the log_2 of the size of our critical buffer. Must be greater than 1.
   *    - filename, the relative path associated with the desired file. Must point to a
   *      valid Ogg Vorbis file.
   * 
   *  Returns:
   *    - A pointer to a heap-allocated VorbisManager, if the inputs are valid.
   *    - Returns nullptr otherwise.
   */ 
  static VorbisManager* GetVorbisManager(int twopow, char* filename);

  /**
   *  Creates a new audio buffer which will be populated by the thread function.
   *  Returns a shared pointer to the buffer.
   * 
   *  Returns:
   *    A shared pointer which references the buffer.
   */ 
  std::shared_ptr<AudioBufferSPSC<float>> CreateBufferInstance();


 private:
  VorbisManager(int twopow, stb_vorbis* file);
};

#endif  // VORBIS_MANAGER_H_