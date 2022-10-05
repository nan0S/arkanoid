#version 330 core

layout(location = 0) in vec2 i_Position;

uniform vec2 translate;

void main()
{
   gl_Position = vec4(i_Position + translate, 0.0f, 1.0f);
}
