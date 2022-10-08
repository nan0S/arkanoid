#version 330 core

layout(location = 0) in vec2 i_position;

out vec2 v_position;

uniform vec2 scale;
uniform vec2 translate;

void main()
{
   gl_Position = vec4(scale * i_position + translate, 0.0f, 1.0f);
   v_position = i_position;
}
