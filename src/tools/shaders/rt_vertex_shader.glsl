#version 410 core

uniform float aspect;
uniform float focalLength;

// perspective * view * world
uniform mat4x4 projectionMatrix;

// Each pixel has screen space coord
layout(location=0) in vec2 pos;

out vec3 rayStart;    // ray start position
out vec3 rayDirection;    // ray start direction
//------------------------------------------------------------------
void main(void) {
    // perspective projection
    vec4 p = projectionMatrix *  vec4(pos.x/aspect, pos.y, 0.0, 1.0);
    rayStart = p.xyz;

    // Subtract camera centre backprojected into cam space
    p -= projectionMatrix * vec4(0.0, 0.0, -focalLength, 1.0);
    rayDirection = vec3(0.0, 0.0, 1.0);

    gl_Position=vec4(pos, 0.0, 1.0);
}
//------------------------------------------------------------------