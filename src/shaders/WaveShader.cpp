#include "shaders/WaveShader.hpp"
#include <iostream>

WaveShader::WaveShader() {
  // create program

  glGenTextures(1, &uDftTex_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, uDftTex_);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 8192, 128, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // define geometry

  // not gonna gen mipmaps because its just a buffer

  
}