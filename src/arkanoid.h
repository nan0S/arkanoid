#ifndef ARKANOID_H
#define ARKANOID_H

#include <stdint.h>
#include <assert.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#include "math.h"
#include "random.h"
#include "shader.h"

#define min(a, b) a < b ? a : b;
#define max(a, b) a < b ? b : a;
#define swap(a, b) { auto &aa = a, &bb = b; auto tmp = aa; aa = bb; bb = tmp; }

#define PI32 3.14159265359f

template<typename Lambda>
struct Scope_guard
{
   Scope_guard(Lambda lambda) : lambda(lambda) {}
   ~Scope_guard() { lambda(); }

   Lambda lambda;
};

#define DO_CONCAT(a, b) a##b
#define CONCAT(a, b) DO_CONCAT(a, b)
#define UNIQUENAME( prefix ) CONCAT(prefix, __COUNTER__)
#define defer Scope_guard UNIQUENAME(sg) = [&]()

#ifndef ARKANOID_SLOW
#define GL_CALL(x) while (glGetError() != GL_NO_ERROR); x; assert(gl_log_error(#x, __FILE__, __LINE__))
#else
#define GL_CALL(x) x
#endif

struct Levels_data
{
   i32 num_levels;
   char **level_paths;
};

struct Level_state
{
   i32 current_level_index;

   char *board;

   i32 num_rows;
   i32 num_cols;
   i32 num_blocks;
};

template<typename T>
struct Array
{
   T *data;
   i32 length;
   i32 capacity;
};

bool
gl_log_error(const char *call, const char *file, int line);

#endif
