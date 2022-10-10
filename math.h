inline float
dot(v2 u, v2 v)
{
   return u.x*v.x + u.y*v.y;
}

inline float
length(v2 v)
{
   return sqrtf(dot(v, v));
}

inline v2
normalized(v2 v)
{
   float len = length(v);
   return v / len;
}

inline v2
v2_from_angle(float a)
{
   return { cosf(a), sinf(a) };
}
