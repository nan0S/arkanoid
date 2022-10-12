#ifndef ARKANOID_H
#define ARKANOID_H

#include <stdint.h>
#include <assert.h>

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

struct Loaded_level
{
   char *board;
   i32 num_rows;
   i32 num_cols;
   i32 num_blocks;
};

enum Load_level_status_code
{
   LOAD_LEVEL_SUCCESS,
   LOAD_LEVEL_OPEN_FILE_ERROR,
   LOAD_LEVEL_UNKNOWN_CHAR_ERROR,
   LOAD_LEVEL_INCONSISTENT_NUM_OF_COLS_ERROR
};

struct Level_state
{
   i32 num_remaining_blocks;
   v2 *block_translations;

   f32 block_half_width;
   f32 block_half_height;

   GLuint vao;
   GLuint vbo;
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

   i32 loaded_level_index;
   Loaded_level loaded_level;
};

bool
gl_log_error(const char *call, const char *file, int line);

void
initialize_game(Game_state *game_state, Paddle *paddle, Ball *ball, Level_state *level_state);

Load_level_status_code
load_level(Loaded_level *loaded_level, i32 level_index, Levels_data *levels_data);
void
free_level(Loaded_level *loaded_level);

void
ball_follow_paddle(Ball *ball, Paddle *paddle);

#endif
