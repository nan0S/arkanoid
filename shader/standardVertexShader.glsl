#version 330 core

layout(location = 0) in vec2 m_position;
layout(location = 1) in vec3 m_color;

uniform vec2 translate;

void main()
{
    gl_Position = vec4(m_position + translate, 0.0f, 1.0f);
}