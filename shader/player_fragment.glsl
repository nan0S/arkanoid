#version 330 core

in vec2 v_UV;

out vec3 f_Color;

uniform vec3 color;

void main()
{
   f_Color = color;
}
