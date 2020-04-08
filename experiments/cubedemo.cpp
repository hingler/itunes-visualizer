// quickly putting a cube on the screen to get myself back up to speed
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "gl/Shader.hpp"

#include <iostream>

const static float vertices[] = {
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] = {
  0, 1, 3
};

const static char* vShader = 
  "#version 410 core\n"
  "precision mediump float;\n"
  "in vec3 aPos;\n"
  "in vec3 aColor;\n"
  "uniform float time;\n"
  "out vec4 vertexColor;\n"
  "void main() {\n"
  " vertexColor = vec4(aColor.rgb, 1.0);\n"
  " gl_Position = vec4(aPos.xyz, 1.0);\n"
  "}\0";

const static char* fShader =
  "#version 410 core\n"
  "out vec4 FragColor;\n"
  "in vec4 vertexColor;\n"
  "void main() {\n"
  " FragColor.rgb = pow(vertexColor.rgb, vec3(1.0 / 2.2));\n" // gamma correction!
  " FragColor.a = 1.0;\n"
  "}\0";

// called on a window size change
void SizeChangeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
  
  // all of this is from LearnOpenGL.com -- again, just getting myself refreshed
  if (!glfwInit()) {
    return 1;
  }

  GLuint proggers;

  Shader::CreateProgram("../experiments/cubedemo.cpp", "../experiments/cubedemo.cpp", &proggers);

  // glad loader created for OGL 4.1
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  // use core profile vs. compat profile (immediate vs. retained iirc)
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // GLFW is our context creator, so we use it to create a window

  // w/h/name, monitor (NULL = windowed mode), share (a window to share resources with)
  GLFWwindow* window = glfwCreateWindow(1024, 768, "creatine", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    return 1;
  }

  // this context is the one we are going to work with
  glfwMakeContextCurrent(window);

  // now we want to load GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "something's up with glad" << std::endl;
    glfwTerminate();
    return 1;
  }
  
  int attribs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribs);
  std::cout << attribs << std::endl;

  // best way to organize:

  // define a function which configures and returns a VAO
  // individual functions create VAOs for a given object
  // or alternatively we can even do that procedurally
  // and accept an array of ordered data pointers and attribute names

  // we now have access to GL functions

  // (no constants, this would be ideal)
  // tells OGL how to map gl coords to screen coords
  glViewport(0, 0, 1024, 768);

  // modify viewport if framebuffer size (window size) changes
  glfwSetFramebufferSizeCallback(window, SizeChangeCallback);

  // while the window is open

  char infolog[512];

  unsigned int ebo;
  glGenBuffers(1, &ebo);

  // setup the vbo/vao
  unsigned int vbo;
  glGenBuffers(1, &vbo);

  // vao stores calls to 
  //  - enablevertexattribarray
  //  - vertexAttribPointer
  // binding it means that we don't have to rebind and re-specify our
  // VBOs -- instead we can just bind the VAO and the state is wrapped up in it
  // EBO binds are stored as well :)
  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // state driven: tell GL that we want to modify vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // stick the data contained in `vertices` into the buffer
  // currently designated as `GL_ARRAY_BUFFER`.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // create shader
  unsigned int prog = glCreateProgram();

  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  // create shader entity and link the script
  glShaderSource(vertShader, 1, &vShader, NULL);
  // compile (duh)
  glCompileShader(vertShader);

  GLint success;

  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertShader, 512, NULL, infolog);
    std::cout << infolog << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, &fShader, NULL);
  glCompileShader(fragShader);

  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragShader, 512, NULL, infolog);
    std::cout << infolog << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // attach to the program
  glAttachShader(prog, vertShader);
  glAttachShader(prog, fragShader);
  // link (thing gcc)
  glLinkProgram(prog);

  glGetProgramiv(prog, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(prog, 512, NULL, infolog);
    std::cout << infolog << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // put these in a function -- this sucks!
  // lots of semantically meaningful variables
  // which end up getting tossed at the end of the ops
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);


  // specify how our vertices are laid out
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  // map index 0 to the vertex positions in the vert shader
  // are these attrib locations bound to the program?
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  // i think so
  glBindAttribLocation(prog, 0, "aPos");
  glBindAttribLocation(prog, 1, "aColor");

  // enable the attrib
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glUseProgram(prog);

  float time = 0.0f;
  int timeloc = glGetUniformLocation(prog, "time");
  // wrapping this would be great actually now that i think about it

  // draw wireframe instead of filling
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  while (!glfwWindowShouldClose(window)) {
    // double buffered -- swap buffers so that we can start working
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glBindVertexArray(vao);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glUniform1f(timeloc, time);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    // checks for input and calls callbacks assc'd with them
    // writing a class which can represent the state of a shader
    // and give access to uniforms, attributes, etc. etc. would be neat!
    glfwPollEvents();
    time += 0.01f;
  }

  // here: the window has been closed
  
  std::cout << "closing..." << std::endl;
  glfwTerminate();
  return 0;
}

void SizeChangeCallback(GLFWwindow* window, int width, int height) {
  std::cout << "new size: " << width << " x " << height << std::endl;
  glViewport(0, 0, width, height);
}