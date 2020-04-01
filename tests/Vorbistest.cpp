
#include "audiohandlers/VorbisManager.hpp"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include "gtest/gtest.h"

#include <string>

// read a vorbis file
// close it
// give a VM the same file
// ensure that it is read correctly via a separate thread (GetBuffer)
// give the buffer to the stub code to verify that the input is correct
// and also that it does not terminate early

const std::string testfile = "../experiments/flap_jack_scream.ogg";

/**
 *  Some assurances on the basic state of a newly constructed manager
 */ 
TEST(VorbisManagerTests, CreateManager) {
  VorbisManager* mgr = VorbisManager::GetVorbisManager(15, testfile);
  ASSERT_NE(mgr, nullptr);
  ASSERT_FALSE(mgr->IsThreadRunning());

  ReadOnlyBuffer* buf = mgr->CreateBufferInstance();
  float** res;
  ASSERT_EQ(buf->Peek_Chunked(1024, &res), 0);

  delete buf;
  delete mgr;
}

TEST(VorbisManagerTests, ParseBuffer) {
  VorbisManager* mgr = VorbisManager::GetVorbisManager(14, testfile);
  ReadOnlyBuffer* buf = mgr->CreateBufferInstance();

  Set_Filename(testfile);

  mgr->StartWriteThread();
  std::cout << "thread started" << std::endl;
  ASSERT_TRUE(mgr->IsThreadRunning());
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  mgr->StopWriteThread();
}