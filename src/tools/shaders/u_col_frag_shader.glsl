#version 330 core
out vec4 FragColor;
void main() {
  FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}