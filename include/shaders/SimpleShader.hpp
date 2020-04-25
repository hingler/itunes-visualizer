#ifndef SIMPLE_SHADER_H_
#define SIMPLE_SHADER_H_

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "shaders/AudioShader.hpp"

/**
 *  Simple fella. I take no pride.
 */ 
class SimpleShader : public AudioShader {
 public:
  SimpleShader();
  void Render(GLFWwindow* window, float* sample_data, size_t length) override;
  const std::string* GetParameterNames() override;
  void SetParameter(const std::string& param_name, std::any value) override;
 private:
  // preallocate these as they will be used often
  const static int BUFFER_SIZE = 8192;
  float real_output[BUFFER_SIZE];
  float imag_output[BUFFER_SIZE];
  float ampl_output[BUFFER_SIZE];

  // uniform buffers
  GLuint uData_;
  GLuint uCoords_;

  // vertex data
  GLuint aPos_;

  // vao
  GLuint vao_;

  // prog
  GLuint prog_;

};

#endif  // SIMPLE_SHADER_H_