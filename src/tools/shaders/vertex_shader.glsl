#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

out vec4 colour;

void main() {
	vec4 worldCoord = modelMatrix * vec4(aPos, 1.0);
	vec4 viewCoord  = viewMatrix  * worldCoord; 
	colour = vec4((aPos + vec3(1)) / 2. , 1);
	gl_Position = projectionMatrix * viewCoord;
}