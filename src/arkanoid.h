#ifndef ARKANOID_H
#define ARKANOID_H

#include <stdint.h>
#include <assert.h>
#include <malloc.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
#include "levels.h"
#include "shaders.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))
#define swap(a, b) { auto &aa = (a), &bb = (b); auto tmp = aa; aa = bb; bb = tmp; }

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

template<typename T>
struct Array
{
   T *data;
   i32 length;
   i32 capacity;

   T* begin() { return data; }
   T* end() { return data + length; }
   T& operator[](i32 index) { return data[index]; }
};

template<typename T>
Array<T>
array_create(i32 length = 0)
{
   Array<T> arr;
   arr.length = length;
   arr.capacity = length;

   if (length)
      arr.data = (T *)malloc(length * sizeof(T));
   else
      arr.data = 0;

   return arr;
}

template<typename T>
void
array_add(Array<T> *arr, T elem)
{
   assert(arr->length <= arr->capacity);

   if (arr->length == arr->capacity)
   {
      if (arr->capacity == 0) arr->capacity = 1;
      else arr->capacity *= 2;
      arr->data = (T *)realloc(arr->data, arr->capacity * sizeof(T));
   }

   arr->data[arr->length++] = elem;
}

template<typename T>
void
array_free(Array<T> arr)
{
   if (arr.data)
      free(arr.data);
}

#ifndef ARKANOID_SLOW
#define GL_CALL(x) while (glGetError() != GL_NO_ERROR); x; assert(gl_log_error(#x, __FILE__, __LINE__))
#else
#define GL_CALL(x) x
#endif

struct Level
{
   i32 num_rows;
   i32 num_cols;
   i32 num_blocks;

   v2 *block_translations;

   f32 block_half_width;
   f32 block_half_height;

   GLuint vao;
   GLuint vbo;
};

struct All_levels_data
{
   Level *levels;
   i32 num_levels;
};

struct Background
{
   GLuint vao;
};

struct Paddle
{
   GLuint vao;

   v2 translate;
   f32 speed;

   f32 body_width;
   f32 body_height;
   f32 body_half_width;
   f32 body_half_height;

   static const i32 NUM_SEGMENTS = 6;
   f32 segment_length;
   f32 segment_bounce_angles[NUM_SEGMENTS];
};

struct Ball
{
   GLuint vao;

   v2 translate;
   f32 speed;
   v2 velocity;

   f32 radius;
   f32 half_radius;
};

struct Game_state
{
   bool paused;
   bool started;

   i32 level_index;
   Level *level;
   i32 num_remaining_blocks;
};

bool
gl_log_error(const char *call, const char *file, int line);

void
initialize_game(Game_state *game_state, Paddle *paddle, Ball *ball);

void
ball_follow_paddle(Ball *ball, Paddle *paddle);

#endif
