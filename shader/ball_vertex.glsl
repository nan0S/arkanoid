#version 330 core

layout(location = 0) in vec2 i_position;

out vec2 v_position;

uniform float radius;
uniform vec2 translate;

void main()
{
   gl_Position = vec4(radius * i_position + translate, 0.0f, 1.0f);
   v_position = i_position;
}
