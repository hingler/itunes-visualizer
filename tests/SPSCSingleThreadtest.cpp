#include "gtest/gtest.h"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include <cmath>

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
  for (int i = 0; i < 32; i++) {

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

}