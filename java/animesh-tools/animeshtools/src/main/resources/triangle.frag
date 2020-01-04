#version 330

out vec4 fragColor;

in   vec4    varying_Color;

void main (void)
{
    fragColor = varying_Color;
}
