#version 330 core
layout (location = 0) in vec4 a_Position;
layout (location = 1) in vec3 a_Color;

out vec4 out_Color;

void main()
{
    gl_Position = a_Position;
    out_Color = a_Color;
}