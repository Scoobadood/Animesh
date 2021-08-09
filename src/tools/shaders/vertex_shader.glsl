#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

void main() {
	vec4 worldCoord = modelMatrix * vec4(aPos, 1.0);
	vec4 viewCoord  = viewMatrix  * worldCoord; 
	gl_Position = projectionMatrix * viewCoord;
}