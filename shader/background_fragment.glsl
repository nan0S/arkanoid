#version 330 core

out vec3 f_Color;

uniform float time;

float constrain(float value,
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
         f_Color = vec3(2.5f/255, 15.0f/255, 31.0f/255);
      else
         f_Color = vec3(18.5f/255, 15.0f/255, 31.0f/255);
   }
   else
   {
      if (x-y > 0)
         f_Color = vec3(34.5f/255, 15.0f/255, 31.0f/255);
      else
         f_Color = vec3(50.5f/255, 15.0f/255, 31.0f/255);
   }

   // float k = abs(cos(time)) * (1.0f - abs(gl_FragCoord.x));
   float k = abs(cos(time)) * (1 - abs(10*gl_FragCoord.x));
   if (k < 0)
      k = 0;
      if (k > 1)
         k = 1;
   k = constrain(k, 0.0f, 1.0f, 0.6f, 1.0f);
   f_Color *= 2.0f * k;
   f_Color = vec3(gl_FragCoord.x, gl_FragCoord.x, gl_FragCoord.x/10);
}
