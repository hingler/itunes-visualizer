#ifndef MULTI_COLOR_SHADER_H
#define MULTI_COLOR_SHADER_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "shaders/AudioShader.hpp"

class MultiColorShader : public AudioShader {
 public:
  MultiColorShader();
  void Render(GLFWwindow* window, float* sample_data, size_t length) override;
  const std::string* GetParameterNames() override;
  void SetParameter(const std::string& param_name, std::any value) override;
 private:

  // prog
  GLuint prog_;

  // tex id for dft data
  GLuint dft_tex_;

  // vao
  GLuint vao_;
};

#endif  // MULTI_COLOR_SHADER_H