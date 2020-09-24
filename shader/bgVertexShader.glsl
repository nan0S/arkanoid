#version 330 core

layout(location = 0) in vec2 vertex_position;
out vec2 position;

void main()
{
    gl_Position = vec4(vertex_position, 0.0f, 1.0f);
    position = vertex_position;
}