#ifndef SHADERS_H
#define SHADERS_H

const char *background_vertex_code = R"FOO(
#version 330 core

layout(location = 0) in vec2 i_position;

out float v_normalized_position_x;

void main()
{
    gl_Position = vec4(i_position, 0.0f, 1.0f);
    v_normalized_position_x = i_position.x;
}
)FOO";

const char *background_fragment_code = R"FOO(
#version 330 core

in float v_normalized_position_x;

out vec4 f_color;

uniform float time;

float
constrain(float value,
   float value_min,
   float value_max,
   float target_min,
   float target_max)
{
   float value_frac = (value - value_min) / (value_max - value_min);
   float target_span = target_max - target_min;
   return target_min + value_frac * target_span;
}

void main()
{
   int a = 50;
   int x = int(gl_FragCoord.x) % a - a/2;
   int y = int(gl_FragCoord.y) % a - a/2;

   if (x+y > 0)
   {
      if (x-y > 0)
         f_color = vec4(2.5f/255, 15.0f/255, 31.0f/255, 1.0f);
      else
         f_color = vec4(18.5f/255, 15.0f/255, 31.0f/255, 1.0f);
   }
   else
   {
      if (x-y > 0)
         f_color = vec4(34.5f/255, 15.0f/255, 31.0f/255, 1.0f);
      else
         f_color = vec4(50.5f/255, 15.0f/255, 31.0f/255, 1.0f);
   }

   float k = abs(cos(time)) * (1.0f - abs(v_normalized_position_x));
   k = constrain(k, 0.0f, 1.0f, 0.6f, 1.0f);
   f_color.rgb *= 2.0f * k;
}
)FOO";

const char *paddle_vertex_code = R"FOO(
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
)FOO";

const char *paddle_fragment_code = R"FOO(
#version 330 core

in vec2 v_position;

out vec4 f_color;

void main()
{
   f_color = vec4(80.0f/255, 120.0f/255, 111.0f/255, 1.0f);
}
)FOO";

const char *ball_vertex_code = R"FOO(
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
)FOO";

const char *ball_fragment_code = R"FOO(
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
)FOO";

const char *block_vertex_code = R"FOO(
#version 330 core

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_translate;

uniform vec2 scale;

void main()
{
   gl_Position = vec4(scale * i_position + i_translate, 0.0f, 1.0f);
}
)FOO";

const char *block_fragment_code = R"FOO(
#version 330 core

out vec4 f_color;

void main()
{
   f_color = vec4(244.0f/255, 192.0f/255, 149.0f/255, 1.0f);
}
)FOO";

#endif
