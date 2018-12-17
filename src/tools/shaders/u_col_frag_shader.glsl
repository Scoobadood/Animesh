#version 330 core
out vec4 FragColor;
uniform float greenShade;
void main() {
  FragColor = vec4(0.0, greenShade, 0.0, 1.0);
}