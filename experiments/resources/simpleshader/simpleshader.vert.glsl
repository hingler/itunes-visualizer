#version 410 core

in vec4 aPos;

void main() {
  // gl fragcoord only
  gl_Position = aPos;
}