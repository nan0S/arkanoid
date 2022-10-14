#ifndef MATH_H
#define MATH_H

#include <math.h>

union v2
{
   struct
   {
      f32 x, y;
   };
   f32 p[2];
};

inline v2
V2(f32 x, f32 y)
{
   return { x, y };
}

inline v2
operator+(v2 u, v2 v)
{
   return { u.x+v.x, u.y+v.y };
}

inline v2
operator-(v2 u, v2 v)
{
   return { u.x-v.x, u.y-v.y };
}

inline v2
operator-(v2 u)
{
   return { -u.x, -u.y };
}

inline v2
operator/(v2 u, v2 v)
{
   return { u.x/v.x, u.y/v.y };
}

inline v2
operator*(f32 x, v2 v)
{
   return { x*v.x, x*v.y };
}

inline v2
operator*(v2 v, f32 x)
{
   return x * v;
}

inline v2
operator/(v2 v, f32 x)
{
   return (1.0f/x) * v;
}

inline v2 &
operator+=(v2 &u, v2 v)
{
   u = u + v;

   return u;
}

inline v2 &
operator-=(v2 &u, v2 v)
{
   u = u - v;

   return u;
}

inline v2 &
operator*=(v2 &u, f32 x)
{
   u = x * u;

   return u;
}

inline v2 &
operator/=(v2 &u, f32 x)
{
   u = u / x;

   return u;
}

inline f32
dot(v2 u, v2 v)
{
   return u.x*v.x + u.y*v.y;
}

inline f32
length(v2 v)
{
   return sqrtf(dot(v, v));
}

inline v2
normalized(v2 v)
{
   f32 len = length(v);
   return v / len;
}

inline v2
v2_of_angle(f32 a)
{
   return { cosf(a), sinf(a) };
}

union v3
{
   struct
   {
      f32 x, y, z;
   };
   struct
   {
      f32 r, g, b;
   };
   f32 p[3];
};

inline v3
V3(f32 x, f32 y, f32 z)
{
   return { x, y, z };
}

#endif
