#include "gtest/gtest.h"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include <cmath>
#include <random>

#include <thread>

class BufferTests : public testing::Test {
 protected:
  AudioBufferSPSC<int16_t>* q;
  const static int32_t power = 12;
  const static int32_t size = 4096;

  void SetUp() override {
    q = new AudioBufferSPSC<int16_t>(power); // buffer with size 4096
  }

  void TearDown() override {
    delete q;
  }
};  // class BufferTests

const int32_t BufferTests::size;
const int32_t thread_read_len = 65536;
const int32_t thread_rw_size = 1024;

// ensure that the empty buffer is according to form
TEST_F(BufferTests, CheckEmptyBuffer) {
  ASSERT_TRUE(q->Empty());
  ASSERT_EQ(q->Size(), 0);

  int16_t* output;

  ASSERT_EQ(q->Peek(32, &output), 0);
}

// perform a single read/write combo
TEST_F(BufferTests, WriteToBuffer) {
  int16_t prepped[256];

  for (int16_t i = 0; i < 256; i++) {
    prepped[i] = i;
  }

  ASSERT_TRUE(q->Write(prepped, 256));

  int16_t* doise;

  doise = q->Read(256);

  for (int16_t i = 0; i < 256; i++) {
    ASSERT_EQ(doise[i], i);
  }

  ASSERT_EQ(q->Peek(32, &doise), 0);
}

// repeatedly fill and empty the buffer
TEST_F(BufferTests, BufferLoopRead) {
  int16_t write_counter = 0;
  int16_t read_counter = 0;
  int16_t buffer[size];
  int16_t* reader;
  size_t peeker;
  for (int i = 0; i < 1024; i++) {

    ASSERT_EQ(q->Size(), 0);

    for (int i = 0; i < size; i++) {
      buffer[i] = write_counter++;
    }

    // ISSUE: when the buffer is full, read and write point to the same position --
    // in this state, size reports as 0.
    //
    // revise the size call a bit to handle this case
    ASSERT_TRUE(q->Write(buffer, size));

    ASSERT_EQ(q->Size(), size);

    peeker = q->Peek(size, &reader);

    ASSERT_EQ(peeker, size);

    reader = q->Read(size);

    ASSERT_FALSE((reader == nullptr));

    for (int i = 0; i < size; i++) {
      ASSERT_EQ(reader[i], read_counter);
      read_counter++;
    }
  }
}

TEST_F(BufferTests, RandomReadWrite) {
  int16_t buffer_sim[size];
  int16_t buffer_size = 0;

  int16_t buffer_alloc[size];

  srand(0);

  int16_t itr;
  int16_t val;

  int16_t* read_result;

  for (int16_t i = 0; i < 1024; i++) {
    // std::cout << "printing iteration " << i << std::endl;
    // std::cout << "initial buffer size: " << buffer_size << std::endl;
    ASSERT_EQ(buffer_size, q->Size());
    itr = rand() % (size - buffer_size);
    for (int16_t j = 0; j < itr; j++) {
      val = (rand() % 65535) - 32768;
      buffer_alloc[j] = val;
      // sim loads front to back, reads front to back
      // readjust after read
      buffer_sim[buffer_size + j] = val;
    }
    // std::cout << "writing " << itr << " elements..." << std::endl;
    ASSERT_TRUE(q->Write(buffer_alloc, itr));
    buffer_size += itr;

    itr = rand() % (buffer_size);
    // std::cout << "reading " << itr << " elements..." << std::endl;
    ASSERT_GE(q->Size(), itr);
    read_result = q->Read(itr);

    // std::cout << "elements read!" << std::endl;

    ASSERT_TRUE(read_result != nullptr);

    // check read
    for (int16_t j = 0; j < itr; j++) {

      ASSERT_EQ(buffer_sim[j], read_result[j]);
    }

    // std::cout << "equality asserted :-)" << std::endl;

    for (int16_t j = itr, k = 0; j < size; j++, k++) {
      buffer_sim[k] = buffer_sim[j];
    }

    buffer_size -= itr;
  }
}

// thread functions
void WriterThread(int16_t* write_contents, AudioBufferSPSC<int16_t>* q) {
  int32_t counter = 0;
  while (counter < thread_read_len) {
    // write thread takes ownership of cache block until next read
    while (!q->Write(write_contents, thread_rw_size));
    counter += thread_rw_size;
    write_contents += thread_rw_size;
  }
  std::cout << "writer finished successfully!" << std::endl;
}

void ReaderThread(int16_t* verify_contents, AudioBufferSPSC<int16_t>* q) {
  int32_t counter = 0;
  int16_t* output;
  while (counter < thread_read_len) {
    while ((output = q->Read(thread_rw_size)) == nullptr);

    // std::cout << "read " << thread_rw_size << " elements" << std::endl;
    // std::cout << q->Size() << std::endl;

    for (int16_t i = 0; i < thread_rw_size; i++) {
      ASSERT_EQ(output[i], verify_contents[i]);
    }

    verify_contents += thread_rw_size;
    counter += thread_rw_size;
  }
}

TEST_F(BufferTests, MultiThreadTest) {

  int16_t contents[thread_read_len];
  for (int i = 0; i < thread_read_len; i++) {
    contents[i] = (rand() % 65536) - 32768;
  }

  std::thread writer(WriterThread, contents, q);
  std::thread reader(ReaderThread, contents, q);

  writer.join();
  reader.join();
}

// TODO: Write a test using the synchronize, getitemsread, etc. calls
// should simulate normal usage with threads as well
// create another read/write pair which will load as space becomes available.
// use a timer to determine what it should synchronize to.
// the read thread will read whenever it is possible to do so

// even better: the demo has three threads: one write and two read
//  the write syncs with a smaller buffer
//  ensure that all values can be read correctly from the larger buffer (peek logic)

// sample: 44100 sample rate
// should be a shared TimeInfo struct between them

const static int contents_length = 705600;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> hrctp;
typedef std::chrono::duration<double, std::ratio<1L, 1L>> SecDur;

struct BufferPair {
  AudioBufferSPSC<uint32_t>* crit;  // the "critical buffer"
  AudioBufferSPSC<uint32_t>* aux;   // some other buffer
  hrctp pt; // origin time
};

void WriteThread(BufferPair* buf, uint32_t contents[]) {
  int offset = 0;
  // write to both buffers

  // get the max write size, and write if its non zero
  while (offset < contents_length) {

    int max_read;
    do {
      max_read = buf->crit->GetMaximumWriteSize();
    } while (max_read == 0);
    // attempts to write beyond buffer end
    max_read = (max_read > (contents_length - offset) ? (contents_length - offset) : max_read);
    while (!buf->crit->Write(contents + offset, max_read));
    // std::cout << "wrote " << max_read << " samples to crit!" << std::endl;
    buf->aux->Write(contents + offset, max_read);
    // std::cout << "wrote " << max_read << " samples to aux!" << std::endl;
    offset += max_read;
    // std::cout << offset << std::endl;
  }
  std::cout << "write thread complete" << std::endl;
}

void ReadThread(AudioBufferSPSC<uint32_t>* buf, uint32_t contents[], hrctp time, int threadct) {
  // read from a singular buffer
  // verify its contents
  int seek_sample = 0;
  int seek_sample_last = 0;
  uint32_t* data;
  while (seek_sample < contents_length) {
    // synchronize based on the sample count
    // seconds
    double dur = SecDur(std::chrono::high_resolution_clock::now() - time).count();
    // "peek" at the next set of values (length returned)
    seek_sample = (144000 * dur);
    // check values against sample count
    // synchro call has no means of reporting whether the operation was successful!
    if (seek_sample > contents_length) {
      break;
    }

    seek_sample_last = seek_sample;

    while (buf->Size() == 0);
    buf->Synchronize(seek_sample);
    int read_size;
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

// idea for implementation: populate the critical buffer to avoid
// discrepencies between buffers
TEST(ThreadBufferTest, MultiThreadVoyage) {
  uint32_t contents[contents_length];
  for (int i = 0; i < contents_length; i++) {
    contents[i] = i;
  }

  BufferPair buf;
  buf.crit = new AudioBufferSPSC<uint32_t>(12); // 4096
  buf.aux = new AudioBufferSPSC<uint32_t>(13);  // 8192
  buf.pt = std::chrono::high_resolution_clock::now();

  std::thread writer(WriteThread, &buf, contents);
  /**
   * writer thread needs some time to catch up
   * synchronize calls update the read head to a different position
   * however, if we make the sync call before we have even written, it fucks everything up
   * 
   */ 
  // std::this_thread::sleep_for(std::chrono::milliseconds(5));
  std::thread reader_one(ReadThread, buf.crit, contents, buf.pt, 1);
  std::thread reader_two(ReadThread, buf.aux, contents, buf.pt, 2);

  writer.join();
  reader_one.join();
  reader_two.join();

  delete buf.crit;
  delete buf.aux;
}