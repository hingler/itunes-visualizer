#include "gtest/gtest.h"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include <cmath>
#include <random>

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

// ensure that the empty buffer is according to form
TEST_F(BufferTests, CheckEmptyBuffer) {
  ASSERT_TRUE(q->Empty());
  ASSERT_EQ(q->Size(), 0);

  int16_t* output;

  ASSERT_EQ(q->Peek(32, output), 0);
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

  ASSERT_EQ(q->Peek(32, doise), 0);
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

    peeker = q->Peek(size, reader);

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
    std::cout << "printing iteration " << i << std::endl;
    std::cout << "initial buffer size: " << buffer_size << std::endl;
    ASSERT_EQ(buffer_size, q->Size());
    itr = rand() % (size - buffer_size);
    for (int16_t j = 0; j < itr; j++) {
      val = (rand() % 65535) - 32768;
      buffer_alloc[j] = val;
      // sim loads front to back, reads front to back
      // readjust after read
      buffer_sim[buffer_size + j] = val;
    }
    std::cout << "writing " << itr << " elements..." << std::endl;
    ASSERT_TRUE(q->Write(buffer_alloc, itr));
    buffer_size += itr;

    itr = rand() % (buffer_size);
    std::cout << "reading " << itr << " elements..." << std::endl;
    ASSERT_GE(q->Size(), itr);
    read_result = q->Read(itr);

    std::cout << "elements read!" << std::endl;

    ASSERT_TRUE(read_result != nullptr);

    // check read
    for (int16_t j = 0; j < itr; j++) {
      ASSERT_EQ(buffer_sim[j], read_result[j]);
    }

    std::cout << "equality asserted :-)" << std::endl;

    for (int16_t j = itr, k = 0; j < size; j++, k++) {
      buffer_sim[k] = buffer_sim[j];
    }

    buffer_size -= itr;
  }
}