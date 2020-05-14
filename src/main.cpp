#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "audiohandlers/VorbisManager.hpp"
#include "audioreaders/VorbisReader.hpp"
#include "portaudio.h"

#include "shaders/WaveShader.hpp"

void SizeChangeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
  // setup the engine
  if (argc < 2) {
    // bag args
    std::cout << "Invalid number of arguments provided." << std::endl;
    argv[1] = "resources/shawty.ogg";
  }

  PaError err = Pa_Initialize();

  if (err != paNoError) {
    return EXIT_FAILURE;
  }

  AudioReader* reader = VorbisReader::GetVorbisReader(argv[1]);

  std::unique_ptr<VorbisManager> vm(VorbisManager::GetVorbisManager(16, reader));
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
  glViewport(0, 0, 512, 256);
  glfwSetFramebufferSizeCallback(window, SizeChangeCallback);
  WaveShader* shader = new WaveShader();

  // todo: use UBO to get all of the sample data in there
  // or do it with a texture lol

  vm->StartWriteThread();

  float** channeldata;
  size_t samples_read;

  glfwSwapInterval(1);

  int framecount = 0;

  while (!glfwWindowShouldClose(window)) {
    framecount++;
    // todo: sometimes synchronization might fail
    rob->Synchronize_Chunked(-0.2);
    samples_read = rob->Peek_Chunked(8192, &channeldata);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->Render(window, channeldata[0], samples_read);
    glfwSwapBuffers(window);
    glfwPollEvents();
    if (!vm->IsThreadRunning()) {
      glfwSetWindowShouldClose(window, 1);
    }
  }

  glfwDestroyWindow(window);

  vm->StopWriteThread();

  delete shader;

  glfwTerminate();
  Pa_Terminate();
}

void SizeChangeCallback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}