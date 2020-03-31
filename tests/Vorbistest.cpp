
#include "audiohandlers/VorbisManager.hpp"
#include "audiohandlers/AudioBufferSPSC.hpp"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
  return 0;
}

// read a vorbis file
// close it
// give a VM the same file
// ensure that it is read correctly via a separate thread (GetBuffer)
// give the buffer to the stub code to verify that the input is correct
// and also that it does not terminate early