#version 410 core

uniform float texOffsetY;
uniform sampler2D dftHistory;

uniform float spaceWidth;
uniform float spaceDepth;

uniform vec3 camera;

uniform mat4 viewMatrix;

in vec4 vectorPos;

out float intensity;
out float z_dist;

const float INTENSITY_MULTIPLIER = 4.0f;

void main() {
  float xSample = 0.125 - (abs(vectorPos.x) / spaceWidth) / 8;
  float ySample = fract((-1.0 / 256) + texOffsetY + (vectorPos.z / spaceDepth));
  intensity = pow(texture(dftHistory, vec2(xSample, ySample)).r, 0.5); // this should be fine

  // iirc interpolation is causing the front-most sample to combine the lastest and earliest dft samples
  // together

  // offseting by 1/256 (1 / 2 * texheight) might fix that
  // but that would require an additional uniform to handle that for an arbitrary texture size

  // update: i did it and its fixed
  z_dist = length(camera - vectorPos.xyz);
  // i think this is right
  gl_Position = viewMatrix * vec4(vectorPos.x, intensity * INTENSITY_MULTIPLIER, vectorPos.zw);

}

