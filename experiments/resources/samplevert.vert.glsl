#version 410 core
precision mediump float;

in vec3 aPos;
in vec3 aColor;

uniform float time;

out vec4 vertexColor;

void main() {
  vertexColor = vec4(aColor.rgb, 1.0);
  gl_Position = vec4(aPos.xyz, 1.0);
}