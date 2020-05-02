#version 410 core

in vec4 aPos;

out vec2 texcoord;

void main() {
  texcoord = (aPos.xy / 2) + vec2(0.5);
  gl_Position = aPos;
}