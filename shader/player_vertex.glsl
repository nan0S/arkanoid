#version 330 core

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec2 i_UV;

out vec2 v_UV;

uniform vec2 translate;

void main()
{
   gl_Position = vec4(i_Position + translate, 0.0f, 1.0f);
   v_UV = i_UV;
}
