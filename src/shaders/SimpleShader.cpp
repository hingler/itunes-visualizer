#include <shaders/SimpleShader.hpp>
#include <gl/GL.hpp>
#include <audiohandlers/DFT.hpp>

#include <iostream>

SimpleShader::SimpleShader() {
  GLuint prog;
  if (!GL::CreateProgram("resources/simpleshader/simpleshader.vert.glsl",
                         "resources/simpleshader/simpleshader.frag.glsl",
                         &prog)) 
  {
    std::cout << "failed to load program" << std::endl;
    // no other choice at this point -- its gonna crash
    exit(EXIT_FAILURE);
  }

  glUseProgram(prog);

  glGenBuffers(1, &aPos);
  glBindBuffer(GL_ARRAY_BUFFER, aPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(boxCoords), boxCoords, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));

  glBindAttribLocation(prog, 0, "aPos");

  glEnableVertexAttribArray(0);
}

void SimpleShader::Render(float* sample_data, unsigned int length) {
  // get highest 2-pow available from length
  uint32_t maxlen = 1;
  while (maxlen <= length) {
    maxlen <<= 1;
  }

  // get max size size as 2 pow
  maxlen >>= 1;

  dft::CalculateDFT(sample_data, real_output, imag_output, maxlen);
  dft::GetAmplitudeArray(real_output, imag_output, ampl_output, maxlen);



}