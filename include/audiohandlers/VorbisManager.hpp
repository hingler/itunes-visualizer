#ifndef VORBIS_MANAGER_H_
#define VORBIS_MANAGER_H_

#include "vorbis/stb_vorbis.h"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include "portaudio.h"

#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

struct TimeInfo {
  TimeInfo();
  TimeInfo(int sample_rate);

  /**
   *  Returns the current sample, or -1 if nothing is currently playing.
   */ 
  int GetCurrentSample() const;

  /**
   *  Returns true if our PA callback thread is running, false otherwise.
   */ 
  bool IsThreadRunning() const;

  /**
   *  Modifies the current sample rate.
   */ 
  void SetSampleRate(int sample_rate);

  /**
   *  Resets the playback epoch to now.
   */ 
  void ResetEpoch();               

  int sample_rate_;
  std::chrono::time_point<std::chrono::high_resolution_clock> playback_epoch_;
 private:
  // mutable keyword: allows for modification within a const function
  // DO NOT OVERUSE!!!
  // it is useful in this case because we need R/W lock access (and only lock access)
  // within const function calls.
  mutable std::shared_mutex info_lock_;
};

/**
 *  A read-only wrapper for our AudioBufferSPSC which gives the user single-thread access
 *  to the audio buffer's contents. Only exposes chunked functions, to ensure that the read
 *  thread does not desynchronize. We don't really care what the write thread does.
 */ 
class ReadOnlyBuffer {
 public:
  /**
   *  All of the following functions wrap the existing functionality for the AudioBufferSPSC class,
   *  with BUFFER_UNIT = float. Please refer to that class for documentation.
   */
  ReadOnlyBuffer(std::shared_ptr<AudioBufferSPSC<float>> buffer, const TimeInfo* info);

  size_t Peek_Chunked(uint32_t framecount, float*** output);

  float** Read_Chunked(uint32_t framecount);

  // use the timeinfo here
  int Synchronize_Chunked();

  int Size();

  ~ReadOnlyBuffer();

  const TimeInfo* info_;

 private:
  std::shared_ptr<AudioBufferSPSC<float>> buffer_;
};

/**
 *  A pair of atomic flags used to facilitate communication between our thread and our vorbis manager. 
 */ 
struct ThreadPacket {
  std::atomic_flag thread_signal;     // flag raised by the thread
  std::atomic_flag vm_signal;         // flag raised by the manager
};

/**
 *  A packet of data sent to our PaCallback.
 */ 
struct CallbackPacket {
  AudioBufferSPSC<float>* buf;      // the buffer which we are reading from (almost certainly the crit buffer)
  std::atomic_flag callback_signal; // the signal used to communicate with the write thread
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
  static VorbisManager* GetVorbisManager(int twopow, std::string filename);

  /**
   *  Creates a heap-allocated read-only buffer instance which can be used to get info
   *  from the thread.
   * 
   *  Returns:
   *    A shared pointer which references the newly created buffer.
   *    If the inputted size is less than the size of our input buffer, returns null.
   */ 
  ReadOnlyBuffer* CreateBufferInstance();

  /**
   *  Starts up the write thread as well as the PortAudio callback, if they are not already running.
   *  
   */ 
  void StartWriteThread();

  /**
   *  Stops the write thread and PortAudio callback if they are already running.
   */ 
  void StopWriteThread();

  /**
   *  Returns whether or not the write thread is running currently.
   */ 
  bool IsThreadRunning();

  /**
   *  Destructor for the VorbisManager.
   */ 
  ~VorbisManager();

 private:
 /**
  *   Private constructor called by GetVorbisManager.
  */ 
  VorbisManager(int twopow, stb_vorbis* file);

  /**
   *  Function which actually does the reading/writing
   */ 
  void WriteThreadFn();

  /**
   *  Prunes expired buffers and performs the call back function on all remaining ones.
   */ 
  void EraseOrCallback(const std::function<void(std::shared_ptr<AudioBufferSPSC<float>>)>&);

  /**
   *  Fills all buffers based on the write capacity of the critical buffer.
   *  Returns whether or not there is more content in the vorbis stream to read.
   *  
   */ 
  bool PopulateBuffers(int write_size);

  // handles cases where the buffer is full when we try to write more shit
  static void FillBufferListCallback(std::shared_ptr<AudioBufferSPSC<float>> buf, float* input, int write_size);

  /**
   *  Callback passed to PortAudio
   */ 
  static int PaCallback(  const void* inputBuffer,
                          void* outputBuffer,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData  );

  // PRIVATE FIELDS

  /**
   *  A list of weak pointers to our buffers. If a user is done with their buffer this
   *  ensures that everything is cleared correctly while still allowing write access.
   */ 
  std::list<std::weak_ptr<AudioBufferSPSC<float>>> buffer_list_;

  /**
   *  The lock associated with all accesses to the buffer list.
   */ 
  std::mutex buffer_list_lock_;

  /**
   *  The "critical" buffer which is used by our PortAudio callback function.
   */ 
  AudioBufferSPSC<float>* critical_buffer_;
  // TODO: Should this in fact be volatile???

  /**
   *  The audiofile which is being managed.
   */ 
  stb_vorbis* audiofile_;

  /**
   *  The sample rate of our vorbis file.
   */ 
  unsigned int sample_rate_;

  /**
   *  The number of channels contained in our vorbis file.
   */ 
  int channel_count_;

  /**
   *  Used by the write thread as free space when reading via vorbis.
   */ 
  float* read_buffer_;

  /**
   *  Thread object representing our write thread.
   */ 
  std::thread write_thread_;

  /**
   *  Determines whether a given thread is running.
   */ 
  std::atomic<bool> run_thread_;

  /**
   *  Two-power used to initialize additional buffers.
   */ 
  int buffer_power_;

  /**
   *  pair of atomic flags used to facilitate communication btwn thread and parent.
   */ 
  ThreadPacket packet;

  /**
   *  TimeInfo object associated with manager.
   */ 
  TimeInfo info;
};

#endif  // VORBIS_MANAGER_H_