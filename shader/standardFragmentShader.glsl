#version 330 core

uniform vec3 color;
out vec3 m_color;

void main()
{
    m_color = color;
}