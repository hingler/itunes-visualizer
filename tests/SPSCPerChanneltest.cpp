// TODO: Write tests which verify that our per-channel read functions work correctly
//       For an arbitrary number of channels (1, 2, 5, n, ...)

#include "gtest/gtest.h"
#include "audiohandlers/AudioBufferSPSC.hpp"

#include <chrono>
#include <thread>

TEST(ChannelBufferTests, SimpleReadWriteTest) {
  uint32_t data[1024];
  for (int i = 0; i < 512; i++) {
    data[2*i] = 4096 - i;
    data[2*i + 1] = i;
  }

  AudioBufferSPSC<uint32_t> buf(12, 2);
  buf.Write(data, 1024);

  std::cout << "Written to buffer!" << std::endl;

  // should have two channels

  uint32_t** result = buf.Read_Chunked(512);
  std::cout << "Reading from buffer..." << std::endl;

  for (int i = 0; i < 512; i++) {
    ASSERT_EQ(result[1][i], data[2 * i + 1]);
    ASSERT_EQ(result[0][i], data[2 * i]);
  }
}

const static int contents_length = 705600;
const static int sample_rate = 576000 * 2;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> hrctp;
typedef std::chrono::duration<double, std::ratio<1L, 1L>> SecDur;

struct BufferPair {
  AudioBufferSPSC<uint32_t>* crit;  // the "critical buffer"
  AudioBufferSPSC<uint32_t>* aux;   // some other buffer
  AudioBufferSPSC<uint32_t>* mono;   // some other buffer
  hrctp pt; // origin time
};

// copied from spsctests
// todo: ensure contents are blocked neatly
void WriteThread(BufferPair* buf, uint32_t contents[]) {
  int offset = 0;

  while (offset < (contents_length * 2)) {

    int max_read;
    do {
      max_read = buf->crit->GetMaximumWriteSize();
    } while (max_read == 0);
    max_read = (max_read > ((contents_length * 2) - offset) ? ((contents_length * 2) - offset) : max_read);
    while (!buf->crit->Write(contents + offset, max_read));
    buf->aux->Write(contents + offset, max_read);
    buf->mono->Write(contents + offset, max_read);
    offset += max_read;
    // std::cout << "offset: " << offset << std::endl;
  }
  std::cout << "write thread complete" << std::endl;
}
void ReadThread_STEREO(AudioBufferSPSC<uint32_t>* buf, uint32_t contents[], hrctp time, int threadct) {
  int seek_sample = 0;
  int seek_sample_last = 0;
  uint32_t** data;
  while (seek_sample < contents_length) {
    double dur = SecDur(std::chrono::high_resolution_clock::now() - time).count();
    // "peek" at the next set of values (length returned)
    seek_sample = (sample_rate * dur);
    // check values against sample count
    // synchro call has no means of reporting whether the operation was successful!
    if (seek_sample >= contents_length) {
      break;
    }

    seek_sample_last = seek_sample;

    // grabs write until it updates
    while (buf->Size() == 0);
    // once it updates, sync
    buf->Synchronize_Chunked(seek_sample);
    int read_size;

    do {
      read_size = buf->Peek_Chunked(1024, &data);
    } while (read_size == 0);
    for (int i = 0; i < read_size; i++) {
      // two threads
      // writing interleaved
      // start should be 2 * seek_sample
      ASSERT_EQ(contents[2 * (seek_sample + i)], data[0][i]);
      ASSERT_EQ(contents[2 * (seek_sample + i) + 1], data[1][i]);
    }

    if (threadct == 1) {
      std::cout << "read " << read_size << " elements" << std::endl;
      std::cout << "read up to frame " << seek_sample + read_size << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  std::cout << "thread is over!" << std::endl;
}

void ReadThread_MONO(AudioBufferSPSC<uint32_t>* buf, uint32_t contents[], hrctp time, int threadct) {
  // read from a singular buffer
  // verify its contents
  int seek_sample = 0;
  int seek_sample_last = 0;
  uint32_t* data;
  while (seek_sample < contents_length * 2) {
    // synchronize based on the sample count
    // seconds
    double dur = SecDur(std::chrono::high_resolution_clock::now() - time).count();
    // "peek" at the next set of values (length returned)
    seek_sample = (sample_rate * 2 * dur);
    // check values against sample count
    // synchro call has no means of reporting whether the operation was successful!
    if (seek_sample >= contents_length * 2) {
      break;
    }

    seek_sample_last = seek_sample;

    // grabs write until it updates
    while (buf->Size() == 0);
    // once it updates, sync
    buf->Synchronize(seek_sample);
    int read_size;
    // peek at the data that's there
    do {
      read_size = buf->Peek(1024, &data);
      // end of buffer: no discerning factor
    } while (read_size == 0);
    for (int i = 0; i < read_size; i++) {
      ASSERT_EQ(contents[i + seek_sample], data[i]);
    }

    if (threadct == 1) {
      std::cout << "read up to sample " << seek_sample + read_size << std::endl;
    }
    // sleep for a stretch
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  
  std::cout << "thread is over!" << std::endl;
}

TEST(ChannelBufferTests, MultiThreadVoyage) {
  uint32_t* contents = new uint32_t[contents_length * 2];
  for (int i = 0; i < contents_length * 2; i += 2) {
    contents[i] = i;
    contents[i + 1] = contents_length + i;
  }

  std::cout << "the" << std::endl;

  BufferPair buf;
  buf.crit = new AudioBufferSPSC<uint32_t>(14, 2);  // 16384 frames in stereo (32768 samples)
  buf.aux = new AudioBufferSPSC<uint32_t>(15, 2);   // 32768 frames in stereo (65536 samples)
  buf.mono = new AudioBufferSPSC<uint32_t>(16, 1);  // 65536 samples in mono

  std::cout << "the" << std::endl;

  ASSERT_EQ(buf.crit->Capacity(), 32768);
  ASSERT_EQ(buf.aux->Capacity(), 65536);

  buf.pt = std::chrono::high_resolution_clock::now();
  std::thread writer(WriteThread, &buf, contents);

  std::thread reader_one(ReadThread_STEREO, buf.crit, contents, buf.pt, 3);
  std::thread reader_two(ReadThread_STEREO, buf.aux, contents, buf.pt, 2);
  std::thread reader_mono(ReadThread_MONO, buf.mono, contents, buf.pt, 1);

  writer.join();
  reader_one.join();
  reader_two.join();
  reader_mono.join();

  delete buf.crit;
  delete buf.aux;
}

// next big
// TODO: Rewrite the VorbisManager.
//       The VorbisManager should handle this Read/Write loop as well as the PA Audio Callback.
//       