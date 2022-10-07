#version 330 core

layout(location = 0) in vec2 i_position;

out float v_normalized_position_x;

void main()
{
    gl_Position = vec4(i_position, 0.0f, 1.0f);
    v_normalized_position_x = i_position.x;
}
