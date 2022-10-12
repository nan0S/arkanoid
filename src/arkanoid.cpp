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
initialize_game(Game_state *game_state, Paddle *paddle, Ball *ball, Level_state *level_state)
{
   paddle->translate = V2(0.0f, -0.86f);

   f32 velocity_angle = random_between(0.25f, 0.75f) * M_PI;
   ball->velocity = v2_of_angle(velocity_angle);
   ball_follow_paddle(ball, paddle);

   game_state->started = false;

   Loaded_level *loaded_level = &game_state->loaded_level;
   level_state->num_remaining_blocks = loaded_level->num_blocks;
   level_state->block_translations = (v2 *)malloc(level_state->num_remaining_blocks * sizeof(v2));

   f32 screen_width = 2.0f;
   f32 between_blocks_padding = 0.01f;

   f32 block_width = (screen_width - (loaded_level->num_cols-1) * between_blocks_padding) / loaded_level->num_cols;
   f32 block_height = 0.05f;

   level_state->block_half_width = 0.5f * block_width;
   level_state->block_half_height = 0.5f * block_height;

   i32 index = 0;
   i32 block_index = 0;

   for (i32 row = 0; row < loaded_level->num_rows; ++row)
   {
      for (i32 col = 0; col < loaded_level->num_cols; ++col)
      {
         if (loaded_level->board[index] == 'X')
         {
            f32 pos_x = -1.0f + (col + 0.5f) * block_width + col * between_blocks_padding;
            f32 pos_y = 1.0f - (row + 0.5f) * block_height - row * between_blocks_padding;

            level_state->block_translations[block_index].x = pos_x;
            level_state->block_translations[block_index].y = pos_y;

            ++block_index;
         }

         ++index;
      }
   }

   GL_CALL(glBindVertexArray(level_state->vao));
   GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, level_state->vbo));
   GL_CALL(glBufferData(GL_ARRAY_BUFFER,
            level_state->num_remaining_blocks * sizeof(v2),
            level_state->block_translations,
            GL_STATIC_DRAW));
}

Load_level_status_code
load_level(Loaded_level *loaded_level, i32 level_index, Levels_data *levels_data)
{
   auto do_load_level = [](Loaded_level *loaded_level, FILE *level_file)
   {
      char *board_it = loaded_level->board;
      char *line_buffer = 0;
      size_t buffer_length = 0;
      size_t line_length;

      defer { free(line_buffer); };

      while ((line_length = getline(&line_buffer, &buffer_length, level_file)) != -1)
      {
         if (line_buffer[line_length-1] == '\n')
            --line_length;

         for (size_t i = 0; i < line_length; ++i)
         {
            char c = line_buffer[i];
            switch (c)
            {
               case 'X':
                  ++loaded_level->num_blocks;
                  break;
               case '.':
                  break;
               default:
                  return LOAD_LEVEL_UNKNOWN_CHAR_ERROR;
            }
         }

         if (loaded_level->num_cols == 0)
            loaded_level->num_cols = line_length;
         else if (loaded_level->num_cols != line_length)
            return LOAD_LEVEL_INCONSISTENT_NUM_OF_COLS_ERROR;

         memcpy(board_it, line_buffer, line_length * sizeof(char));
         board_it += line_length;

         ++loaded_level->num_rows;
      }

      return LOAD_LEVEL_SUCCESS;
   };

   assert(0 <= level_index);
   assert(level_index < levels_data->num_levels);

   char *level_path = levels_data->level_paths[level_index];
   FILE *level_file = fopen(level_path, "r");

   if (!level_file)
      return LOAD_LEVEL_OPEN_FILE_ERROR;

   defer { fclose(level_file); };

   fseek(level_file, SEEK_SET, SEEK_END);
   long level_file_length = ftell(level_file);
   rewind(level_file);

   loaded_level->num_rows = 0;
   loaded_level->num_cols = 0;
   loaded_level->num_blocks = 0;
   loaded_level->board = (char *)malloc(level_file_length * sizeof(char));

   auto ret = do_load_level(loaded_level, level_file);
   if (ret != LOAD_LEVEL_SUCCESS)
      free(loaded_level->board);

   return ret;
}

void
free_level(Loaded_level *loaded_level)
{
   free(loaded_level->board);
}

void
ball_follow_paddle(Ball *ball, Paddle *paddle)
{
   f32 eps = 0.001f;
   ball->translate.x = paddle->translate.x;
   ball->translate.y = paddle->translate.y + paddle->body_half_height + ball->half_radius + eps;
}

i32
main()
{
   Levels_data levels_data;
   {
      const char *levels_dir = "../levels/";
      size_t levels_dir_length = strlen(levels_dir);

      FILE *all_levels_file = fopen("../levels/all_levels.txt", "r");
      if (!all_levels_file)
      {
         fprintf(stderr, "Failed to open file with all levels: '%s'.\n", strerror(errno));
         return EXIT_FAILURE;
      }

      defer { fclose(all_levels_file); };

      char *line_buffer = 0;
      size_t buffer_length = 0;
      size_t line_length;

      i32 level_paths_array_capacity = 1;
      levels_data.num_levels = 0;
      levels_data.level_paths = (char **)malloc(level_paths_array_capacity * sizeof(char *));

      while ((line_length = getline(&line_buffer, &buffer_length, all_levels_file)) != -1)
      {
         if (line_buffer[line_length-1] == '\n')
            line_buffer[--line_length] = 0;

         size_t level_path_length = levels_dir_length + line_length;
         char *level_path = (char *)malloc((level_path_length+1) * sizeof(char));
         memcpy(level_path, levels_dir, levels_dir_length * sizeof(char));
         strcpy(level_path + levels_dir_length, line_buffer);

         if (levels_data.num_levels == level_paths_array_capacity)
         {
            level_paths_array_capacity *= 2;
            levels_data.level_paths = (char **)realloc(levels_data.level_paths, level_paths_array_capacity * sizeof(char *));
         }

         assert(levels_data.num_levels < level_paths_array_capacity);

         levels_data.level_paths[levels_data.num_levels++] = level_path;
      }

      free(line_buffer);

      if (levels_data.num_levels < level_paths_array_capacity)
         levels_data.level_paths = (char **)realloc(levels_data.level_paths, levels_data.num_levels * sizeof(char *));
   }

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

   GLuint bg_shader = Graphics::load_shaders(
         "../shader/background_vertex.glsl",
         "../shader/background_fragment.glsl");

   if (!bg_shader)
   {
      fprintf(stderr, "Failed to load background shader.\n");
      return EXIT_FAILURE;
   }

   GLuint player_shader = Graphics::load_shaders(
         "../shader/player_vertex.glsl",
         "../shader/player_fragment.glsl");

   if (!player_shader)
   {
      fprintf(stderr, "Failed to load player shader.\n");
      return EXIT_FAILURE;
   }

   GLuint ball_shader = Graphics::load_shaders(
         "../shader/ball_vertex.glsl",
         "../shader/ball_fragment.glsl");

   if (!ball_shader)
   {
      fprintf(stderr, "Failed to load ball shader.\n");
      return EXIT_FAILURE;
   }

   GLuint block_shader = Graphics::load_shaders(
         "../shader/block_vertex.glsl",
         "../shader/block_fragment.glsl");

   if (!block_shader)
   {
      fprintf(stderr, "Failed to load block shader.\n");
      return EXIT_FAILURE;
   }

   GL_CALL(GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time"));
   GL_CALL(GLint player_shader_translate_uniform = glGetUniformLocation(player_shader, "translate"));
   GL_CALL(GLint player_shader_scale_uniform = glGetUniformLocation(player_shader, "scale"));
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

   Paddle paddle;
   {
      glGenVertexArrays(1, &paddle.vao);
      glBindVertexArray(paddle.vao);

      GLuint vbo;
      GL_CALL(glGenBuffers(1, &vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      paddle.translate = V2(0.0f, 0.0f);
      paddle.speed = 2.0f;

      paddle.body_width = 0.2f;
      paddle.body_height = 0.035f;
      paddle.body_half_width = 0.5f * paddle.body_width;
      paddle.body_half_height = 0.5f * paddle.body_height;
      paddle.segment_length = paddle.body_width / paddle.NUM_SEGMENTS;

      f32 deg_to_rad = PI32 / 180;
      paddle.segment_bounce_angles[0] = 140 * deg_to_rad;
      paddle.segment_bounce_angles[1] = 115 * deg_to_rad;
      paddle.segment_bounce_angles[2] = 100 * deg_to_rad;
      paddle.segment_bounce_angles[3] = 80 * deg_to_rad;
      paddle.segment_bounce_angles[4] = 65 * deg_to_rad;
      paddle.segment_bounce_angles[5] = 40 * deg_to_rad;
   }

   Ball ball;
   {
      // TODO(hobrzut): Maybe remove duplication in the future.
      GL_CALL(glGenVertexArrays(1, &ball.vao));
      GL_CALL(glBindVertexArray(ball.vao));

      GLuint vbo;
      GL_CALL(glGenBuffers(1, &vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      ball.translate = V2(0.0f, 0.0f);
      ball.speed = 1.6f;
      ball.velocity = V2(0.0f, 0.0f);

      ball.radius = 0.025f;
      ball.half_radius = 0.5f * ball.radius;
   }

   Game_state game_state;
   {
      game_state.paused = false;

      auto status_code = load_level(&game_state.loaded_level, 0, &levels_data);
      assert(status_code == LOAD_LEVEL_SUCCESS);
      game_state.loaded_level_index = 0;
   }

   Level_state level_state;
   {
      GL_CALL(glGenVertexArrays(1, &level_state.vao));
      GL_CALL(glBindVertexArray(level_state.vao));

      GLuint square_vbo;
      GL_CALL(glGenBuffers(1, &square_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, square_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      GL_CALL(glGenBuffers(1, &level_state.vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, level_state.vbo));

      GL_CALL(glEnableVertexAttribArray(1));
      GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0));
      GL_CALL(glVertexAttribDivisor(1, 1));
   }

   initialize_game(&game_state, &paddle, &ball, &level_state);

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

      f32 begin_time = glfwGetTime();

      {
         bool game_over = false;
         bool restart_requested = false;
         bool next_level = false;

         if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            game_state.started = true;
         if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            restart_requested = true;

         // TODO(hobrzut): Remove that.
         if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            ball.speed = 0.01f;
         else
            ball.speed = 1.6f;

         bg_time += delta_time;

         f32 paddle_velocity_x = 0.0f;
         if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            paddle_velocity_x -= paddle.speed;
         if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            paddle_velocity_x += paddle.speed;
         paddle.translate.x += delta_time * paddle_velocity_x;

         f32 max_left = -1.0f + paddle.body_half_width;
         f32 max_right = 1.0f - paddle.body_half_width;
         if (paddle.translate.x > max_right)
            paddle.translate.x = max_right;
         if (paddle.translate.x < max_left)
            paddle.translate.x = max_left;

         if (game_state.started)
         {
            v2 new_ball_translate = ball.translate + delta_time * ball.speed * ball.velocity;
            bool ball_disturbed = false;

            if (new_ball_translate.x < -1.0f || new_ball_translate.x > 1.0f)
            {
               ball.velocity.x = -ball.velocity.x;
               ball_disturbed = true;
            }
            if (new_ball_translate.y > 1.0f)
            {
               ball.velocity.y = -ball.velocity.y;
               ball_disturbed = true;
            }
            if (new_ball_translate.y < -1.1f - ball.half_radius)
               game_over = true;

            for (i32 i = 0; i < level_state.num_remaining_blocks; ++i)
            {
               v2 ball_block_diff = new_ball_translate - level_state.block_translations[i];
               f32 abs_diff_x = abs(ball_block_diff.x);
               f32 abs_diff_y = abs(ball_block_diff.y);
               f32 extent_x = level_state.block_half_width + ball.half_radius;
               f32 extent_y = level_state.block_half_height + ball.half_radius;

               if (abs_diff_x <= extent_x && abs_diff_y <= extent_y)
               {
                  f32 scaled_x = abs_diff_x / (level_state.block_half_width + ball.half_radius);
                  f32 scaled_y = abs_diff_y / (level_state.block_half_height + ball.half_radius);

                  if (scaled_x < scaled_y)
                     ball.velocity.y = -ball.velocity.y;
                  else
                     ball.velocity.x = -ball.velocity.x;

                  ball_disturbed = true;

                  swap(level_state.block_translations[i], level_state.block_translations[--level_state.num_remaining_blocks]);

                  if (level_state.num_remaining_blocks == 0)
                     next_level = true;
                  else
                  {
                     GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, level_state.vbo));
                     GL_CALL(glBufferData(GL_ARRAY_BUFFER,
                              level_state.num_remaining_blocks * sizeof(v2),
                              level_state.block_translations,
                              GL_STATIC_DRAW));
                  }

                  break;
               }
            }

            if (ball.translate.y >= paddle.translate.y)
            {
               v2 ball_player_diff = new_ball_translate - paddle.translate;
               if (abs(ball_player_diff.x) <= paddle.body_half_width + ball.half_radius &&
                   abs(ball_player_diff.y) <= paddle.body_half_height + ball.half_radius)
               {
                  f32 bounce_x = ball_player_diff.x + paddle.body_half_width;

                  i32 segment_index = (i32)(bounce_x / paddle.segment_length);
                  if (segment_index < 0) segment_index = 0;
                  if (segment_index >= paddle.NUM_SEGMENTS) segment_index = paddle.NUM_SEGMENTS-1;

                  f32 bounce_angle = paddle.segment_bounce_angles[segment_index];
                  ball.velocity = v2_of_angle(bounce_angle);

                  ball_disturbed = true;
               }
            }

            if (!ball_disturbed)
               ball.translate = new_ball_translate;

            // Moving player could bump into the ball. In that case disconnect two bodies
            // by just teleporting the ball a little further.
            if (ball.translate.y >= paddle.translate.y)
            {
               v2 ball_player_diff = ball.translate - paddle.translate;
               if (abs(ball_player_diff.x) <= paddle.body_half_width + ball.half_radius &&
                   abs(ball_player_diff.y) <= paddle.body_half_height + ball.half_radius)
               {
                  v2 tv = ball_player_diff / ball.velocity;
                  assert(tv.x >= 0.0f && tv.y >= 0.0f);

                  f32 eps = 0.001f;
                  f32 t = min(tv.x, tv.y) + eps;
                  ball.translate += t * ball.velocity;
               }
            }
         }
         else
         {
            ball_follow_paddle(&ball, &paddle);
         }

         GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

         // Draw background.
         GL_CALL(glUseProgram(bg_shader));
         GL_CALL(glBindVertexArray(bg.vao));
         GL_CALL(glUniform1f(bg_shader_time_uniform, bg_time));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw player.
         GL_CALL(glUseProgram(player_shader));
         GL_CALL(glBindVertexArray(paddle.vao));
         GL_CALL(glUniform2f(player_shader_scale_uniform, paddle.body_half_width, paddle.body_half_height));
         GL_CALL(glUniform2f(player_shader_translate_uniform, paddle.translate.x, paddle.translate.y));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw ball.
         GL_CALL(glUseProgram(ball_shader));
         GL_CALL(glBindVertexArray(ball.vao));
         GL_CALL(glUniform1f(ball_shader_radius_uniform, ball.half_radius));
         GL_CALL(glUniform2f(ball_shader_translate_uniform, ball.translate.x, ball.translate.y));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw blocks.
         GL_CALL(glUseProgram(block_shader));
         GL_CALL(glBindVertexArray(level_state.vao));
         GL_CALL(glUniform2f(block_shader_scale_uniform, level_state.block_half_width, level_state.block_half_height));
         GL_CALL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, level_state.num_remaining_blocks));

         glfwSwapBuffers(window);

         if (game_over || restart_requested || next_level)
         {
            if (next_level)
            {
               free_level(&game_state.loaded_level);
               free(level_state.block_translations);

               i32 next_level_index = (game_state.loaded_level_index+1) % levels_data.num_levels;
               auto status = load_level(&game_state.loaded_level, next_level_index, &levels_data);
               assert(status == LOAD_LEVEL_SUCCESS);
               game_state.loaded_level_index = next_level_index;
            }

            initialize_game(&game_state, &paddle, &ball, &level_state);
         }
      }

      delta_time = glfwGetTime() - begin_time;

      printf("\rFrame took %.3fms", delta_time * 1000);
   }

   return EXIT_SUCCESS;
}
