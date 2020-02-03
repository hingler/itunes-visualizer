#include "gtest/gtest.h"
#include "audiohandlers/AudioQueue.hpp"

#include <iostream>

// todo: create a concurrency demo once the workflow for that is good

class AudioTest : public testing::Test {
 protected:
  AudioQueue<int16_t>* q;
  static const int16_t FILL_SIZE = 256;
  void SetUp() override {
    q = new AudioQueue<int16_t>(1024);
  }

  void TearDown() override {
    delete q;
  }
};

const int16_t AudioTest::FILL_SIZE;

TEST_F(AudioTest, CheckEmptyBuffer) {
  ASSERT_TRUE(q->Empty());
  ASSERT_EQ(q->Size(), 0);
  bool success;
  int16_t testunit = q->Pop(success);
  ASSERT_FALSE(success);
}  // CheckEmptyBuffer

TEST_F(AudioTest, WriteToBuffer) {
  int16_t prepped[FILL_SIZE];
  
  for (int16_t i = 0; i < FILL_SIZE; i++) {
    prepped[i] = i ^ 0xF8B3;
  }
  q->Write(prepped, FILL_SIZE);

  for (int16_t i = 0; i < FILL_SIZE; i++) {
    ASSERT_EQ(prepped[i], q->Pop());
  }
}  // WriteToBuffer

TEST_F(AudioTest, CheckMultipleFills) {
  const int ITER_COUNT = 16;

  int16_t rand_data[FILL_SIZE];
  unsigned int seed = 0;

  bool success;

  for (int16_t i = 0; i < ITER_COUNT; i++) {
    for (int16_t j = 0; j < FILL_SIZE; j++) {
      rand_data[i] = rand_r(&seed);
    }

    q->Write(rand_data, FILL_SIZE);
    ASSERT_EQ(FILL_SIZE, q->Size());

    for (int16_t j = 0; j < FILL_SIZE; j++) {
      // have it leap around a bit
      // errors are being generated here and i am not sure whyZzz
      ASSERT_EQ(rand_data[j], q->Pop(success));
      ASSERT_TRUE(success);
    }

    ASSERT_EQ(0, q->Size());
  }
}  // CheckMultipleFills

TEST_F(AudioTest, CircularReadWrite) {
  int16_t rand_data[8192];
  unsigned int seed = 0;

  for (int16_t i = 0; i < 8192; i++) {
    rand_data[i] = rand_r(&seed);
  }

  int16_t read_cursor = 0;
  int16_t write_cursor = 0;

  int cur_amt;

  // read a random amount, write a random amount, ensure correctness.
  while (read_cursor < 8192 && write_cursor < 8192) {
    cur_amt = rand_r(&seed) % (1024 - (write_cursor - read_cursor));
    cur_amt = (cur_amt + write_cursor > 8192 ? 8192 - write_cursor : cur_amt);

    q->Write(rand_data + write_cursor, cur_amt);
    write_cursor += cur_amt;
    
    ASSERT_EQ(q->Size(), write_cursor - read_cursor);

    cur_amt = rand_r(&seed) % q->Size();
    for (int i = 0; i < cur_amt; i++) {
      ASSERT_EQ(rand_data[read_cursor], q->Pop());
      read_cursor++;
    }

    ASSERT_EQ(q->Size(), write_cursor - read_cursor);
  }

  while (read_cursor < 8192) {
    ASSERT_EQ(q->Pop(), rand_data[read_cursor++]);
  }

  ASSERT_EQ(read_cursor, write_cursor);
}  // CircularReadWrite
