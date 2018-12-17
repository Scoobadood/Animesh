#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
out vec3 vertexColour;
void main() {
	vec3 newPos = vec3(aPos.x + 0.5, aPos.yz);
  	gl_Position = vec4(newPos, 1.0);
	vertexColour = newPos;
}