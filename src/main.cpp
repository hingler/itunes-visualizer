#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "audiohandlers/VorbisManager.hpp"
#include "portaudio.h"

#include "shaders/SimpleShader.hpp"

void SizeChangeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
  // setup the engine
  if (argc < 2) {
    // bag args
    std::cout << "Invalid number of arguments provided." << std::endl;
    return EXIT_FAILURE;
  }

  Pa_Initialize();

  std::unique_ptr<VorbisManager> vm(VorbisManager::GetVorbisManager(16, argv[1]));
  if (vm == nullptr) {
    // invalid filename or some other error
    return EXIT_FAILURE;
  }

  // vm is up
  // main thread is render thread
  // no other threads should be necessary
  std::unique_ptr<ReadOnlyBuffer> rob(vm->CreateBufferInstance());

  if (!glfwInit()) {
    std::cout << "failed to initialize glfw" << std::endl;
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  GLFWwindow* window = glfwCreateWindow(512, 256, "pizza planet", NULL, NULL);

  if (window == NULL) {
    glfwTerminate();
    std::cout << "failed to create window" << std::endl;
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glViewport(0, 0, 1024, 768);
  glfwSetFramebufferSizeCallback(window, SizeChangeCallback);
  SimpleShader shader;

  vm->StartWriteThread();

  float** channeldata;
  size_t samples_read;

  glfwSwapInterval(1);

  int framecount = 0;

  while (!glfwWindowShouldClose(window)) {
    framecount++;
    rob->Synchronize_Chunked(-0.2);
    samples_read = rob->Peek_Chunked(8192, &channeldata);
    shader.Render(window, channeldata[0], samples_read);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  vm->StopWriteThread();
  glfwTerminate();
  
}

void SizeChangeCallback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}