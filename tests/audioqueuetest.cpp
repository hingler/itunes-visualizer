#include "gtest/gtest.h"
#include "audiohandlers/AudioQueue.hpp"

#include <iostream>

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
}

TEST_F(AudioTest, WriteToBuffer) {
  int16_t prepped[FILL_SIZE];
  
  for (int16_t i = 0; i < FILL_SIZE; i++) {
    prepped[i] = i ^ 0xF8B3;
  }
  q->Write(prepped, FILL_SIZE);

  for (int16_t i = 0; i < FILL_SIZE; i++) {
    ASSERT_EQ(prepped[i], q->Pop());
  }
}

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
      ASSERT_EQ(rand_data[j], q->Pop(success));
    }

    ASSERT_EQ(0, q->Size());
  }
}