#version 410 core
precision mediump float;

in vec3 aPos;
in vec2 aTex;

uniform float time;

out vec2 texCoord;

void main() {
  texCoord = aTex + vec2(time, sin(time));
  gl_Position = vec4(aPos.xyz, 1.0);
}