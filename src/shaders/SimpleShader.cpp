#include "shaders/SimpleShader.hpp"
#include "gl/GL.hpp"
#include "audiohandlers/DFT.hpp"

#include <iostream>

SimpleShader::SimpleShader() {
  if (!GL::CreateProgram("resources/simpleshader/simpleshader.vert.glsl",
                         "resources/simpleshader/simpleshader.frag.glsl",
                         &prog_)) 
  {
    std::cout << "failed to load program" << std::endl;
    std::cin.ignore();
    // no other choice at this point -- its gonna crash
    exit(EXIT_FAILURE);
  }

  glGenBuffers(1, &aPos_);
  glBindBuffer(GL_ARRAY_BUFFER, aPos_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(boxCoords), boxCoords, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));

  glBindAttribLocation(prog_, 0, "aPos");

  glEnableVertexAttribArray(0);

  glUseProgram(prog_);

  uData_ = glGetUniformLocation(prog_, "uData");
  uCoords_ = glGetUniformLocation(prog_, "uCoords");
}

void SimpleShader::Render(GLFWwindow* window, float* sample_data, size_t length) {
  // get highest 2-pow available from length
  uint32_t maxlen = 1;
  while (maxlen <= length) {
    maxlen <<= 1;
  }

  // get max size size as 2 pow
  maxlen >>= 1;

  dft::CalculateDFT(sample_data, real_output, imag_output, maxlen);
  dft::GetAmplitudeArray(real_output, imag_output, ampl_output, maxlen, false);

  glUseProgram(prog_);
  glBindVertexArray(vao_);

  int screencoords[2];

  glfwGetFramebufferSize(window, &screencoords[0], &screencoords[1]);

  glUniform1fv(glGetUniformLocation(prog_, "uData"), 960, ampl_output);   // only the bottom 256
  glUniform2f(glGetUniformLocation(prog_, "uCoords"), static_cast<float>(screencoords[0]), static_cast<float>(screencoords[1]));  // screencoords
  glUniform1f(glGetUniformLocation(prog_, "time"), glfwGetTime());

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

const std::string* SimpleShader::GetParameterNames() {
  return nullptr;
}

void SimpleShader::SetParameter(const std::string& param_name, std::any value) {
  // does nothing right now since there are no parameters to speak of 
}

SimpleShader::~SimpleShader() {
  glDeleteBuffers(1, &aPos_);
  glDeleteProgram(prog_);
  glDeleteVertexArrays(1, &vao_);
}