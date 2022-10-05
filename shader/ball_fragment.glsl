#version 330 core

in vec2 v_UV;

out vec3 f_Color;

uniform vec3 color;

void main()
{
   if (dot(uv, uv) <= 1.0f)
      f_Color = color;
   else
      f_Color.a = 0.0f;
}
