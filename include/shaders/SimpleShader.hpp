#ifndef SIMPLE_SHADER_H_
#define SIMPLE_SHADER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shaders/AudioShader.hpp>

class SimpleShader : public AudioShader {
 public:
  void Render(float* sample_data, unsigned int length) override;
  const std::string* GetParameterNames() override;
  void SetParameter(const std::string& param_name, std::any value) override;
 private:
  // preallocate these as they will be used often
  const static int BUFFER_SIZE = 4096;
  float real_output[BUFFER_SIZE];
  float imag_output[BUFFER_SIZE];
  float ampl_output[BUFFER_SIZE];

  // uniform buffers
  GLuint uData;
  GLuint uCoords;
  GLuint aPos;

  // vao
  GLuint vao;

};

#endif  // SIMPLE_SHADER_H_