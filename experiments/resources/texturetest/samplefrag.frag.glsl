#version 410 core

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D tex;
uniform sampler2D textwo;

uniform float time;

void main() {
  FragColor.rgb = mix(texture(tex, texCoord).rgb, texture(textwo, texCoord).rgb, vec3((cos(time * 1.414f) + 1.0f) * 0.5f));
  FragColor.a = 1.0;
}