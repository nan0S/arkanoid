#ifndef RANDOM_H
#define RANDOM_H

// TODO(hobrzut): Remove.
#include <stdlib.h>

inline f32
random_unilateral()
{
   return (f32)rand() / RAND_MAX;
}

inline f32
random_between(f32 min, f32 max)
{
   f32 t = random_unilateral();
   return (1-t) * min + t * max;
}

#endif
