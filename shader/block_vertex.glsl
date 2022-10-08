#version 330 core

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_translate;

uniform vec2 scale;

void main()
{
   gl_Position = vec4(scale * i_position + i_translate, 0.0f, 1.0f);
}
