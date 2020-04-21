#version 410 core

precision highp float;
precision mediump int;

out vec4 FragColor;

const int EQ_SIZE = 960;

uniform float uData[EQ_SIZE];    // transform data
uniform vec2 uCoords;

uniform float time;

void main() {
  // 0 to 1
  vec2 pos = gl_FragCoord.xy / uCoords;
  int index = int(pos.x * EQ_SIZE);
  FragColor = vec4(vec3(pos.y < sqrt(uData[index]) / 16), 1.0);
}