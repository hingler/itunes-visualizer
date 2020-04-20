#version 410 core

precision highp float;
precision mediump int;

const int EQ_SIZE = 512;

uniform float[EQ_SIZE] uData;    // transform data
uniform vec2 uCoords;

void main() {
  // 0 to 1
  vec2 pos = gl_FragCoord.xy / uCoords;
  int sample = floor(pos.x / EQ_SIZE);
  gl_FragColor = vec4(vec3(pos.y < (log(uData[sample]) / 4)), 1.0);
}