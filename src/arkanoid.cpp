#include "arkanoid.h"
#include "shader.cpp"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void
window_resize_handler(GLFWwindow *, int width, int height)
{
   int size = min(width, height);
   int width_offset = (width - size) / 2;
   int height_offset = (height - size) / 2;

   glViewport(width_offset, height_offset, size, size);
}

void
glfw_error_callback(int code, const char *desc)
{
   fprintf(stderr, "GLFW error [%d]: %s\n", code, desc);
}

bool
gl_log_error(const char *call, const char *file, int line)
{
   GLenum err;
   bool no_error = true;

   while ((err = glGetError()) != GL_NO_ERROR)
   {
      const char *msg = (const char *)gluErrorString(err);
      fprintf(stderr, "OpenGL error [%d] after '%s' at %s:%d: %s\n", err, call, file, line, msg);
      no_error = false;
   }

   return no_error;
}

void
change_level(Game_state *game_state, i32 new_level_index)
{
   Level *new_level = &game_state->all_levels_data.levels[new_level_index];

   game_state->level_index = new_level_index;
   game_state->level = new_level;
   game_state->num_blocks_left = new_level->num_blocks;

   GL_CALL(glBindVertexArray(new_level->vao));
   GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, new_level->vbo));
   GL_CALL(glBufferSubData(GL_ARRAY_BUFFER,
            0,
            new_level->num_blocks * (sizeof(v2) + sizeof(v3)),
            new_level->translations));

   restart_level_maintaining_destroyed_blocks(game_state);
}

void
restart_level_maintaining_destroyed_blocks(Game_state *game_state)
{
   game_state->started = false;
   game_state->wait_event = WAIT_EVENT_NONE;

   game_state->paddle.translate = V2(0.0f, -0.86f);
   game_state->paddle.body_half_width = 0.5f * Paddle::NORMAL_BODY_WIDTH;

   game_state->ball.speed = Ball::NORMAL_SPEED;
   f32 velocity_angle = random_between(0.25f, 0.75f) * PI32;
   game_state->ball.velocity = v2_of_angle(velocity_angle);
   ball_follow_paddle(&game_state->ball, &game_state->paddle);

   game_state->collectables.num_collectables = 0;
}

void
ball_follow_paddle(Ball *ball, Paddle *paddle)
{
   f32 eps = 0.001f;
   ball->translate.x = paddle->translate.x;
   ball->translate.y = paddle->translate.y + paddle->body_half_height + ball->half_radius + eps;
}

void
set_paddle_width(Paddle *paddle, f32 new_width)
{
   paddle->body_half_width = 0.5f * new_width;
}

void
add_collectable(Collectables *collectables, Collectable_type type, v2 translation)
{
   v3 color;

   switch (type)
   {
      case COLLECTABLE_TYPE_LONG_PADDLE: {
         color = Colors::GREEN;
      } break;
      case COLLECTABLE_TYPE_SHORT_PADDLE: {
         color = Colors::BLUE;
      } break;
      case COLLECTABLE_TYPE_FAST_BALL: {
         color = Colors::YELLOW;
      } break;
      case COLLECTABLE_TYPE_SLOW_BALL: {
         color = Colors::PURPLE;
      } break;
      case COLLECTABLE_TYPE_BALL_SPLIT: {
         color = Colors::PURPLE;
      } break;

      case COLLECTABLE_TYPE_NONE:
         assert(false);
   }

   i32 index = collectables->num_collectables;
   assert(index < collectables->MAX_NUM_COLLECTABLES);

   collectables->types[index] = type;
   collectables->translations[index] = translation;
   collectables->colors[index] = color;

   collectables->num_collectables = index+1;
}

void
remove_collectable(Collectables *collectables, i32 index)
{
   assert(0 <= index && index < collectables->num_collectables);

   i32 end_index = collectables->num_collectables-1;
   swap(collectables->types[index], collectables->types[end_index]);
   swap(collectables->translations[index], collectables->translations[end_index]);
   swap(collectables->colors[index], collectables->colors[end_index]);

   collectables->num_collectables = end_index;
}

i32
main()
{
   if (!glfwInit())
   {
      fprintf(stderr, "Failed to initialize GLFW.\n");
      return EXIT_FAILURE;
   }

   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow *window;
   {
      i32 width = 1080;
      i32 height = 1080;
      window = glfwCreateWindow(width, height, "Arkanoid", 0, 0);

      if (!window)
      {
         fprintf(stderr, "Failed to open GLFW window.\n");
         return EXIT_FAILURE;
      }

      window_resize_handler(window, width, height);

      glfwMakeContextCurrent(window);
      glfwSetFramebufferSizeCallback(window, window_resize_handler);
      glfwSetErrorCallback(glfw_error_callback);
      glfwSwapInterval(0);

      GL_CALL(glClearColor(7.0f/255, 30.0f/255, 34.0f/255, 1.0f));
      GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
      GL_CALL(glEnable(GL_BLEND));
   }

   if (glewInit() != GLEW_OK)
   {
      fprintf(stderr, "Failed to initialize GLEW.\n");
      return EXIT_FAILURE;
   }

   GLuint bg_shader = Graphics::compile_shaders(background_vertex_code, background_fragment_code);
   if (!bg_shader)
   {
      fprintf(stderr, "Failed to load background shader.\n");
      return EXIT_FAILURE;
   }

   GLuint paddle_shader = Graphics::compile_shaders(paddle_vertex_code, paddle_fragment_code);
   if (!paddle_shader)
   {
      fprintf(stderr, "Failed to load paddle shader.\n");
      return EXIT_FAILURE;
   }

   GLuint ball_shader = Graphics::compile_shaders(ball_vertex_code, ball_fragment_code);
   if (!ball_shader)
   {
      fprintf(stderr, "Failed to load ball shader.\n");
      return EXIT_FAILURE;
   }

   GLuint block_shader = Graphics::compile_shaders(block_vertex_code, block_fragment_code);
   if (!block_shader)
   {
      fprintf(stderr, "Failed to load block shader.\n");
      return EXIT_FAILURE;
   }

   GL_CALL(GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time"));
   GL_CALL(GLint paddle_shader_translate_uniform = glGetUniformLocation(paddle_shader, "translate"));
   GL_CALL(GLint paddle_shader_scale_uniform = glGetUniformLocation(paddle_shader, "scale"));
   GL_CALL(GLint ball_shader_translate_uniform = glGetUniformLocation(ball_shader, "translate"));
   GL_CALL(GLint ball_shader_radius_uniform = glGetUniformLocation(ball_shader, "radius"));
   GL_CALL(GLint block_shader_scale_uniform = glGetUniformLocation(block_shader, "scale"));

   // TODO(hobrzut): Maybe get rid of square and use instancing.
   v2 square[] = {
      { -1.0f, -1.0f },
      { -1.0f,  1.0f },
      {  1.0f, -1.0f },
      {  1.0f,  1.0f },
   };

   Background bg;
   {
      GL_CALL(glGenVertexArrays(1, &bg.vao));
      GL_CALL(glBindVertexArray(bg.vao));

      GLuint vbo;
      GL_CALL(glGenBuffers(1, &vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
   }

   Game_state game_state;

   All_levels_data *all_levels_data = &game_state.all_levels_data;
   {
      all_levels_data->num_levels = num_levels;
      all_levels_data->levels = (Level *)malloc(all_levels_data->num_levels * sizeof(Level));

      for (i32 level_index = 0; level_index < all_levels_data->num_levels; ++level_index)
      {
         Level *level = &all_levels_data->levels[level_index];
         level->num_rows = 0;
         level->num_cols = 0;
         level->num_blocks = 0;

         const char *level_symbols = all_levels[level_index];
         i32 level_symbols_index = 0;
         char symbol;
         i32 num_cols = 0;

         // Allow that so that board is more readable.
         if (level_symbols[0] == BOARD_SYMBOL_NEW_ROW)
            ++level_symbols;

         while ((symbol = level_symbols[level_symbols_index++]))
         {
            if (symbol != BOARD_SYMBOL_NEW_ROW)
               ++num_cols;

            switch (symbol)
            {
               case BOARD_SYMBOL_EMPTY: break;

               case BOARD_SYMBOL_BLOCK_NORMAL:
               case BOARD_SYMBOL_BLOCK_LONG_PADDLE:
               case BOARD_SYMBOL_BLOCK_SHORT_PADDLE:
               case BOARD_SYMBOL_BLOCK_FAST_BALL:
               case BOARD_SYMBOL_BLOCK_SLOW_BALL: {
                  ++level->num_blocks;
               } break;

               case BOARD_SYMBOL_NEW_ROW: {
                  if (!level->num_cols)
                     level->num_cols = num_cols;
                  else if (num_cols != level->num_cols)
                  {
                     fprintf(stderr, "Inconsitent number of columns in level %d text (row %d, expected %d, actual %d).\n",
                           level_index+1,
                           level->num_rows+1,
                           level->num_cols,
                           num_cols);
                     return EXIT_FAILURE;
                  }

                  ++level->num_rows;
                  num_cols = 0;
               } break;

               default: {
                  fprintf(stderr, "Unknown character '%c' in level %d text.\n", symbol, level_index+1);
                  return EXIT_FAILURE;
               }
            }
         }

         if (num_cols)
         {
            fprintf(stderr, "Expected newline at the end of level %d text.\n", level_index+1);
            return EXIT_FAILURE;
         }

         size_t total_num_bytes = level->num_blocks * (sizeof(Collectable_type) + sizeof(v2) + sizeof(v3));
         level->allocated_memory = malloc(total_num_bytes);

         level->translations = (v2 *)level->allocated_memory;
         level->colors = (v3 *)(level->translations + level->num_blocks);
         level->collectable_types = (Collectable_type *)(level->colors + level->num_blocks);

         f32 screen_width = 2.0f;
         f32 between_blocks_padding = 0.01f;
         f32 block_width = (screen_width - (level->num_cols-1) * between_blocks_padding) / level->num_cols;
         f32 block_height = 0.05f;

         level->block_half_width = 0.5f * block_width;
         level->block_half_height = 0.5f * block_height;

         i32 index = 0;
         i32 block_index = 0;

         for (i32 row = 0; row < level->num_rows; ++row)
         {
            for (i32 col = 0; col < level->num_cols+1; ++col)
            {
               bool is_block = true;

               switch (level_symbols[index])
               {
                  case BOARD_SYMBOL_BLOCK_NORMAL: {
                     level->collectable_types[block_index] = COLLECTABLE_TYPE_NONE;
                     level->colors[block_index] = Colors::RED;
                  } break;
                  case BOARD_SYMBOL_BLOCK_LONG_PADDLE: {
                     level->collectable_types[block_index] = COLLECTABLE_TYPE_LONG_PADDLE;
                     level->colors[block_index] = Colors::GREEN;
                  } break;
                  case BOARD_SYMBOL_BLOCK_SHORT_PADDLE: {
                     level->collectable_types[block_index] = COLLECTABLE_TYPE_SHORT_PADDLE;
                     level->colors[block_index] = Colors::BLUE;
                  } break;
                  case BOARD_SYMBOL_BLOCK_FAST_BALL: {
                     level->collectable_types[block_index] = COLLECTABLE_TYPE_FAST_BALL;
                     level->colors[block_index] = Colors::YELLOW;
                  } break;
                  case BOARD_SYMBOL_BLOCK_SLOW_BALL: {
                     level->collectable_types[block_index] = COLLECTABLE_TYPE_SLOW_BALL;
                     level->colors[block_index] = Colors::PURPLE;
                  } break;

                  default: {
                     is_block = false;
                  } break;
               }

               if (is_block)
               {
                  f32 pos_x = -1.0f + (col + 0.5f) * block_width + col * between_blocks_padding;
                  f32 pos_y = 1.0f - (row + 0.5f) * block_height - row * between_blocks_padding;

                  level->translations[block_index] = V2(pos_x, pos_y);

                  ++block_index;
               }

               ++index;
            }
         }

         GL_CALL(glGenVertexArrays(1, &level->vao));
         GL_CALL(glBindVertexArray(level->vao));

         GLuint square_vbo;
         GL_CALL(glGenBuffers(1, &square_vbo));
         GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, square_vbo));
         GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

         GL_CALL(glEnableVertexAttribArray(0));
         GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

         // translations ..., colors ...
         GLsizeiptr translations_size = level->num_blocks * sizeof(v2);
         GLsizeiptr colors_size = level->num_blocks * sizeof(v3);
         GLsizeiptr allocation_size = translations_size + colors_size;
         GLsizeiptr translations_offset = 0;
         GLsizeiptr colors_offset = translations_size;

         GL_CALL(glGenBuffers(1, &level->vbo));
         GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, level->vbo));
         GL_CALL(glBufferData(GL_ARRAY_BUFFER, allocation_size, 0, GL_STATIC_DRAW));

         level->vbo_allocated_size = allocation_size;

         GL_CALL(glEnableVertexAttribArray(1));
         GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void *)translations_offset));
         GL_CALL(glVertexAttribDivisor(1, 1));

         GL_CALL(glEnableVertexAttribArray(2));
         GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)colors_offset));
         GL_CALL(glVertexAttribDivisor(2, 1));
      }
   }

   Paddle *paddle = &game_state.paddle;
   {
      paddle->translate = V2(0.0f, 0.0f);
      paddle->speed = 2.0f;

      f32 body_width = 0.2f;
      f32 body_height = 0.035f;

      paddle->body_half_width = 0.5f * body_width;
      paddle->body_half_height = 0.5f * body_height;
      paddle->segment_length = body_width / paddle->NUM_SEGMENTS;

      f32 deg_to_rad = PI32 / 180;
      paddle->segment_bounce_angles[0] = 140 * deg_to_rad;
      paddle->segment_bounce_angles[1] = 115 * deg_to_rad;
      paddle->segment_bounce_angles[2] = 100 * deg_to_rad;
      paddle->segment_bounce_angles[3] = 80 * deg_to_rad;
      paddle->segment_bounce_angles[4] = 65 * deg_to_rad;
      paddle->segment_bounce_angles[5] = 40 * deg_to_rad;

      glGenVertexArrays(1, &paddle->vao);
      glBindVertexArray(paddle->vao);

      GLuint square_vbo;
      GL_CALL(glGenBuffers(1, &square_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, square_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
   }

   Ball *ball = &game_state.ball;
   {
      ball->translate = V2(0.0f, 0.0f);
      ball->speed = Ball::NORMAL_SPEED;
      ball->velocity = V2(0.0f, 0.0f);

      ball->radius = 0.025f;
      ball->half_radius = 0.5f * ball->radius;

      // TODO(hobrzut): Maybe remove duplication in the future.
      GL_CALL(glGenVertexArrays(1, &ball->vao));
      GL_CALL(glBindVertexArray(ball->vao));

      GLuint square_vbo;
      GL_CALL(glGenBuffers(1, &square_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, square_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
   }

   Collectables *collectables = &game_state.collectables;
   {
      collectables->num_collectables = 0;
      collectables->fall_speed = 0.5f;

      f32 body_width = 0.1f;
      f32 body_height = 0.05f;
      collectables->body_half_width = 0.5f * body_width;
      collectables->body_half_height = 0.5f * body_height;

      GL_CALL(glGenVertexArrays(1, &collectables->vao));
      GL_CALL(glBindVertexArray(collectables->vao));

      GLuint square_vbo;
      GL_CALL(glGenBuffers(1, &square_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, square_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      GLsizeiptr translations_size = collectables->MAX_NUM_COLLECTABLES * sizeof(v2);
      GLsizeiptr colors_size = collectables->MAX_NUM_COLLECTABLES * sizeof(v3);
      GLsizeiptr allocation_size = translations_size + colors_size;
      GLsizeiptr translations_offset = 0;
      GLsizeiptr colors_offset = translations_size;

      GL_CALL(glGenBuffers(1, &collectables->vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, collectables->vbo));
      // TODO(hobrzut): Change to DYNAMIC_DRAW?
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, allocation_size, 0, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(1));
      GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void *)translations_offset));
      GL_CALL(glVertexAttribDivisor(1, 1));

      GL_CALL(glEnableVertexAttribArray(2));
      GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)colors_offset));
      GL_CALL(glVertexAttribDivisor(2, 1));
   }

   game_state.paused = false;
   game_state.lives_left = game_state.INITIAL_LIVES;
   change_level(&game_state, 0);

   f32 bg_time = 0.0f;
   f32 delta_time = 0.0f;
   i32 p_button_last_state = GLFW_RELEASE;

   while (!glfwWindowShouldClose(window))
   {
      if (game_state.paused) glfwWaitEvents();
      else glfwPollEvents();

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
         break;

      i32 p_button_state = glfwGetKey(window, GLFW_KEY_P);
      if (p_button_last_state == GLFW_RELEASE &&  p_button_state == GLFW_PRESS)
         game_state.paused = !game_state.paused;
      p_button_last_state = p_button_state;

      if (game_state.paused)
         continue;

      bg_time += delta_time;

      f32 begin_time = glfwGetTime();

      if (game_state.wait_event == WAIT_EVENT_NONE)
      {
         bool game_over = false;
         bool restart_requested = false;
         bool level_complete = false;

         if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            game_state.started = true;
         if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            restart_requested = true;

         f32 paddle_velocity_x = 0.0f;
         if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            paddle_velocity_x -= paddle->speed;
         if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            paddle_velocity_x += paddle->speed;
         paddle->translate.x += delta_time * paddle_velocity_x;

         f32 max_left = -1.0f + paddle->body_half_width;
         f32 max_right = 1.0f - paddle->body_half_width;
         if (paddle->translate.x > max_right)
            paddle->translate.x = max_right;
         if (paddle->translate.x < max_left)
            paddle->translate.x = max_left;

         if (game_state.started)
         {
            // Update collectables.
            for (i32 i = 0; i < collectables->num_collectables;)
            {
               v2 *c_translate = &collectables->translations[i];
               c_translate->y -= delta_time * collectables->fall_speed;

               v2 collectable_paddle_diff = *c_translate - paddle->translate;
               if (abs(collectable_paddle_diff.x) <= collectables->body_half_width + paddle->body_half_width &&
                   abs(collectable_paddle_diff.y) <= collectables->body_half_height + paddle->body_half_height)
               {
                  switch (collectables->types[i])
                  {
                     case COLLECTABLE_TYPE_LONG_PADDLE: {
                        paddle->body_half_width = 0.5f * Paddle::LONG_BODY_WIDTH;
                     } break;
                     case COLLECTABLE_TYPE_SHORT_PADDLE: {
                        paddle->body_half_width = 0.5f * Paddle::SHORT_BODY_WIDTH;
                     } break;
                     case COLLECTABLE_TYPE_FAST_BALL: {
                        ball->speed = Ball::FAST_SPEED;
                     } break;
                     case COLLECTABLE_TYPE_SLOW_BALL: {
                        ball->speed = Ball::SLOW_SPEED;
                     } break;
                     case COLLECTABLE_TYPE_BALL_SPLIT: {
                     } break;

                     default:
                        assert(false);
                  }

                  remove_collectable(collectables, i);
               }
               else if (c_translate->y <= -1.0f - collectables->body_half_height - 0.05f)
                  remove_collectable(collectables, i);
               else
                  ++i;
            }

            // Update ball.
            v2 new_ball_translate = ball->translate + delta_time * ball->speed * ball->velocity;
            bool ball_disturbed = false;

            if (new_ball_translate.x < -1.0f || new_ball_translate.x > 1.0f)
            {
               ball->velocity.x = -ball->velocity.x;
               ball_disturbed = true;
            }
            if (new_ball_translate.y > 1.0f)
            {
               ball->velocity.y = -ball->velocity.y;
               ball_disturbed = true;
            }
            if (new_ball_translate.y < -1.1f - ball->half_radius)
               game_over = true;

            // Check collisions of ball and board blocks.
            Level *level = game_state.level;
            for (i32 i = 0; i < game_state.num_blocks_left; ++i)
            {
               v2 ball_block_diff = new_ball_translate - level->translations[i];
               f32 abs_diff_x = abs(ball_block_diff.x);
               f32 abs_diff_y = abs(ball_block_diff.y);
               f32 extent_x = level->block_half_width + ball->half_radius;
               f32 extent_y = level->block_half_height + ball->half_radius;

               if (abs_diff_x <= extent_x && abs_diff_y <= extent_y)
               {
                  f32 scaled_x = abs_diff_x / (level->block_half_width + ball->half_radius);
                  f32 scaled_y = abs_diff_y / (level->block_half_height + ball->half_radius);

                  if (scaled_x < scaled_y)
                     ball->velocity.y = -ball->velocity.y;
                  else
                     ball->velocity.x = -ball->velocity.x;

                  ball_disturbed = true;

                  if (level->collectable_types[i] != COLLECTABLE_TYPE_NONE)
                     add_collectable(collectables, level->collectable_types[i], level->translations[i]);

                  i32 end_index = game_state.num_blocks_left-1;
                  swap(level->collectable_types[i], level->collectable_types[end_index]);
                  swap(level->translations[i], level->translations[end_index]);
                  swap(level->colors[i], level->colors[end_index]);

                  game_state.num_blocks_left = end_index;

                  if (game_state.num_blocks_left == 0)
                     level_complete = true;
                  else
                  {
                     GL_CALL(glBindVertexArray(level->vao));
                     GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, level->vbo));
                     GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, level->vbo_allocated_size, level->allocated_memory));
                  }

                  break;
               }
            }

            if (ball->translate.y >= paddle->translate.y)
            {
               v2 ball_player_diff = new_ball_translate - paddle->translate;
               if (abs(ball_player_diff.x) <= paddle->body_half_width + ball->half_radius &&
                   abs(ball_player_diff.y) <= paddle->body_half_height + ball->half_radius)
               {
                  f32 bounce_x = ball_player_diff.x + paddle->body_half_width;

                  i32 segment_index = (i32)(bounce_x / paddle->segment_length);
                  if (segment_index < 0) segment_index = 0;
                  if (segment_index >= paddle->NUM_SEGMENTS) segment_index = paddle->NUM_SEGMENTS-1;

                  f32 bounce_angle = paddle->segment_bounce_angles[segment_index];
                  ball->velocity = v2_of_angle(bounce_angle);

                  ball_disturbed = true;
               }
            }

            if (!ball_disturbed)
               ball->translate = new_ball_translate;

            // Moving player could bump into the ball-> In that case disconnect two bodies
            // by just teleporting the ball a little further.
            if (ball->translate.y >= paddle->translate.y)
            {
               v2 ball_player_diff = ball->translate - paddle->translate;
               if (abs(ball_player_diff.x) <= paddle->body_half_width + ball->half_radius &&
                   abs(ball_player_diff.y) <= paddle->body_half_height + ball->half_radius)
               {
                  v2 tv = ball_player_diff / ball->velocity;
                  assert(tv.x >= 0.0f && tv.y >= 0.0f);

                  f32 eps = 0.001f;
                  f32 t = min(tv.x, tv.y) + eps;
                  ball->translate += t * ball->velocity;
               }
            }

            // Update collectables' buffers.
            GL_CALL(glBindVertexArray(collectables->vao));
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, collectables->vbo));
            GL_CALL(glBufferSubData(GL_ARRAY_BUFFER,
                     0,
                     collectables->num_collectables * sizeof(v2),
                     collectables->translations));
            GL_CALL(glBufferSubData(GL_ARRAY_BUFFER,
                     collectables->MAX_NUM_COLLECTABLES * sizeof(v2),
                     collectables->num_collectables * sizeof(v3),
                     collectables->colors));

         }
         else
         {
            ball_follow_paddle(ball, paddle);
         }

         if (restart_requested)
         {
            game_state.lives_left = game_state.INITIAL_LIVES;
            change_level(&game_state, game_state.level_index);
         }
         else if (game_over)
         {
            game_state.wait_event = WAIT_EVENT_GAME_OVER;
            game_state.wait_time_left = 0.7f;
         }
         else if (level_complete)
         {
            game_state.wait_event = WAIT_EVENT_NEXT_LEVEL;
            game_state.wait_time_left = 1.0f;
         }
      }
      else
      {
         game_state.wait_time_left -= delta_time;
         if (game_state.wait_time_left <= 0.0f)
         {
            switch (game_state.wait_event)
            {
               case WAIT_EVENT_NEXT_LEVEL: {
                  i32 next_level_index = game_state.level_index + 1;

                  // Game complete.
                  if (next_level_index == all_levels_data->num_levels)
                     break;

                  ++game_state.lives_left;
                  change_level(&game_state, next_level_index);
               } break;

               case WAIT_EVENT_GAME_OVER: {
                  if (game_state.lives_left-- == 0)
                  {
                     game_state.lives_left = game_state.INITIAL_LIVES;
                     change_level(&game_state, game_state.level_index);
                  }
                  else
                  {
                     restart_level_maintaining_destroyed_blocks(&game_state);
                  }
               } break;

               case WAIT_EVENT_NONE: assert(false);
            }
         }
      }

      GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

      // Draw background.
      GL_CALL(glUseProgram(bg_shader));
      GL_CALL(glBindVertexArray(bg.vao));
      GL_CALL(glUniform1f(bg_shader_time_uniform, bg_time));
      GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

      // Draw paddle.
      GL_CALL(glUseProgram(paddle_shader));
      GL_CALL(glBindVertexArray(paddle->vao));
      GL_CALL(glUniform2f(paddle_shader_scale_uniform, paddle->body_half_width, paddle->body_half_height));
      GL_CALL(glUniform2f(paddle_shader_translate_uniform, paddle->translate.x, paddle->translate.y));
      GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

      // Draw blocks.
      GL_CALL(glUseProgram(block_shader));
      GL_CALL(glBindVertexArray(game_state.level->vao));
      GL_CALL(glUniform2f(block_shader_scale_uniform, game_state.level->block_half_width, game_state.level->block_half_height));
      GL_CALL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, game_state.num_blocks_left));

      // Draw collectables.
      GL_CALL(glUseProgram(block_shader));
      GL_CALL(glBindVertexArray(collectables->vao));
      GL_CALL(glUniform2f(block_shader_scale_uniform, collectables->body_half_width, collectables->body_half_height));
      GL_CALL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, collectables->num_collectables));

      // Draw ball.
      GL_CALL(glUseProgram(ball_shader));
      GL_CALL(glBindVertexArray(ball->vao));
      GL_CALL(glUniform1f(ball_shader_radius_uniform, ball->half_radius));
      GL_CALL(glUniform2f(ball_shader_translate_uniform, ball->translate.x, ball->translate.y));
      GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

      glfwSwapBuffers(window);

      delta_time = glfwGetTime() - begin_time;

      printf("\rFrame took %.3fms", delta_time * 1000);
   }

   return EXIT_SUCCESS;
}
