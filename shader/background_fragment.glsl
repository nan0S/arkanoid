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
