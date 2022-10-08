#version 330 core

in vec2 v_position;

out vec4 f_color;

void main()
{
   if (dot(v_position, v_position) <= 1.0f)
      f_color = vec4(244.0f/255, 192.0f/255, 149.0f/255, 1.0f);
   else
      f_color = vec4(0.0f);
}
