#include <string>

/**
 *  A common interface implemented so that we can read multiple types of files via a single unified interface
 */ 
class AudioReader {
 public:

  /**
   *  Constructs a new AudioReader which reads from the passed-in file.
   */ 
  AudioReader(std::string file);

  /**
   *  Get some number of frames from the sample file, interleaved.
   *  Left, then right, then the rest.
   *  @param count - The number of samples we wish to read.
   *  @param output - the buffer where we will place the final output
   */ 
  virtual bool GetSamplesInterleaved(int count, float* output) = 0;

  /**
   *  Returns the sample rate of the desired file.
   */ 
  virtual int GetSampleRate() = 0;

  /**
   *  Returns the number of channels in the desired file.
   */ 
  virtual int GetChannelCount() = 0;

  /**
   *  Seeks to a given sample. Most likely only used to seek to start
   *  but it seems like it would be a good idea to have :)
   */ 
  virtual void Seek(int sample) = 0;
};