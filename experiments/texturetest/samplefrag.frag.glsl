#version 410 core

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D tex;
uniform sampler2D textwo;

void main() {
  FragColor.rgb = pow(texture(tex, texCoord).rgb * texture(textwo, texCoord).rgb, vec3(2.2 / 2.2));
  FragColor.a = 1.0;
}