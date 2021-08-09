#version 330

uniform mat4    uniform_Projection;// Incoming data used by
in vec3  attribute_Position;// the vertex shader
in vec4  attribute_Color;// uniform and attributes

out vec4    varying_Color;// Outgoing varying data
void main(void)
{
    varying_Color = attribute_Color;
    gl_Position = uniform_Projection * vec4(attribute_Position, 1.0);
}