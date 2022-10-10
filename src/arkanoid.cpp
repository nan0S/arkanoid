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

struct Level_info
{
   i32 num_levels;
   char **level_paths;
};

i32
main()
{
   Level_info level_info;
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
      level_info.num_levels = 0;
      level_info.level_paths = (char **)malloc(level_paths_array_capacity * sizeof(char *));

      while ((line_length = getline(&line_buffer, &buffer_length, all_levels_file)) != -1)
      {
         if (line_buffer[line_length-1] == '\n')
            line_buffer[--line_length] = 0;

         size_t level_path_length = levels_dir_length + line_length;
         char *level_path = (char *)malloc((level_path_length+1) * sizeof(char));
         memcpy(level_path, levels_dir, levels_dir_length * sizeof(char));
         strcpy(level_path + levels_dir_length, line_buffer);

         if (level_info.num_levels == level_paths_array_capacity)
         {
            level_paths_array_capacity *= 2;
            level_info.level_paths = (char **)realloc(level_info.level_paths, level_paths_array_capacity * sizeof(char *));
         }

         assert(level_info.num_levels < level_paths_array_capacity);

         level_info.level_paths[level_info.num_levels++] = level_path;
      }

      free(line_buffer);

      level_info.level_paths = (char **)realloc(level_info.level_paths, level_info.num_levels * sizeof(char *));
   }

   char *board;
   i32 num_rows;
   i32 num_cols;
   i32 initial_num_blocks;
   {
      if (level_info.num_levels == 0)
      {
         fprintf(stderr, "No levels specified.\n");
         return EXIT_FAILURE;
      }

      char *level_path = level_info.level_paths[0];
      FILE *level_file = fopen(level_path, "r");

      if (!level_file)
      {
         fprintf(stderr, "Failed to load level '%s': '%s'.", level_path, strerror(errno));
         return EXIT_FAILURE;
      }

      defer { fclose(level_file); };

      fseek(level_file, SEEK_SET, SEEK_END);
      long level_file_length = ftell(level_file);
      rewind(level_file);

      board = (char *)malloc(level_file_length * sizeof(char));
      num_rows = 0;
      num_cols = 0;
      initial_num_blocks = 0;

      char *board_it = board;
      char *line_buffer = 0;
      size_t buffer_length = 0;
      size_t line_length;

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
                  ++initial_num_blocks;
                  break;
               case '.':
                  break;
               default:
                  fprintf(stderr, "Level file '%s' is broken: 'unknown character %c'.\n", level_path, c);
                  return EXIT_FAILURE;
            }
         }

         if (num_cols == 0)
            num_cols = line_length;
         else if (num_cols != line_length)
         {
            fprintf(stderr, "Level file '%s' is broken: 'inconsistent number of columns'.\n", level_path);
            return EXIT_FAILURE;
         }

         memcpy(board_it, line_buffer, line_length * sizeof(char));
         board_it += line_length;

         ++num_rows;
      }

      free(line_buffer);
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

   GLuint bg_vao;
   {
      GL_CALL(glGenVertexArrays(1, &bg_vao));
      GL_CALL(glBindVertexArray(bg_vao));

      GLuint bg_vbo;
      GL_CALL(glGenBuffers(1, &bg_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bg_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
   }

   GLuint player_vao;
   v2 player_translate;
   f32 player_speed = 2.0f;
   f32 player_body_width = 0.2f;
   f32 player_body_height = 0.035f;
   f32 player_body_half_width = 0.5f * player_body_width;
   f32 player_body_half_height = 0.5f * player_body_height;
   const i32 PLAYER_SEGMENTS = 6;
   f32 player_segment_length = player_body_width / PLAYER_SEGMENTS;
   f32 player_segment_bounce_angles[PLAYER_SEGMENTS] = { 140, 115, 100, 80, 65, 40 };
   {
      glGenVertexArrays(1, &player_vao);
      glBindVertexArray(player_vao);

      GLuint player_vbo;
      GL_CALL(glGenBuffers(1, &player_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, player_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      for (i32 i = 0; i < PLAYER_SEGMENTS; ++i)
         player_segment_bounce_angles[i] *= PI32 / 180;
   }

   GLuint ball_vao;
   f32 ball_radius = 0.025f;
   f32 ball_half_radius = 0.5f * ball_radius;
   v2 ball_translate;
   f32 ball_speed = 1.6f;
   v2 ball_velocity;
   {
      // TODO(hobrzut): Maybe remove duplication in the future.
      GL_CALL(glGenVertexArrays(1, &ball_vao));
      GL_CALL(glBindVertexArray(ball_vao));

      GLuint vbo;
      GL_CALL(glGenBuffers(1, &vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
   }

   GLuint board_vao;
   GLuint board_vbo;
   i32 num_blocks;
   f32 block_half_width;
   f32 block_half_height;
   v2 *block_translations;
   {
      num_blocks = initial_num_blocks;
      block_translations = (v2 *)malloc(num_blocks * sizeof(v2));

      f32 screen_width = 2.0f;
      f32 screen_height = 2.0f;
      f32 between_blocks_padding = 0.01f;

      f32 block_width = (screen_width - (num_cols-1) * between_blocks_padding) / num_cols;
      f32 block_height = 0.05f;

      block_half_width = 0.5f * block_width;
      block_half_height = 0.5f * block_height;

      i32 index = 0;
      i32 block_index = 0;

      for (i32 row = 0; row < num_rows; ++row)
      {
         for (i32 col = 0; col < num_cols; ++col)
         {
            if (board[index] == 'X')
            {
               f32 pos_x = -1.0f + (col + 0.5f) * block_width + col * between_blocks_padding;
               f32 pos_y = 1.0f - (row + 0.5f) * block_height - row * between_blocks_padding;

               block_translations[block_index].x = pos_x;
               block_translations[block_index].y = pos_y;

               ++block_index;
            }

            ++index;
         }
      }

      GL_CALL(glGenVertexArrays(1, &board_vao));
      GL_CALL(glBindVertexArray(board_vao));

      GLuint square_vbo;
      GL_CALL(glGenBuffers(1, &square_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, square_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      GL_CALL(glGenBuffers(1, &board_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, board_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, num_blocks * sizeof(v2), block_translations, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(1));
      GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0));
      GL_CALL(glVertexAttribDivisor(1, 1));
   }

   f32 bg_time = 0.0f;
   f32 delta_time = 0.0f;
   bool paused = false;
   i32 p_button_last_state = GLFW_RELEASE;
   bool started;

   auto ball_follow_player = [&ball_translate,
      &player_translate,
      player_body_half_height,
      ball_half_radius]()
   {
      f32 eps = 0.001f;
      ball_translate.x = player_translate.x;
      ball_translate.y = player_translate.y + player_body_half_height + ball_half_radius + eps;
   };

   auto initialize_game = [&started,
      &player_translate,
      &ball_translate,
      &ball_velocity,
      &ball_follow_player,
      ball_half_radius,
      player_body_half_height]()
   {
      started = false;

      player_translate = { 0.0f, -0.86f };

      ball_follow_player();

      f32 velocity_angle = random_between(0.25f*M_PI, 0.75f*M_PI);
      ball_velocity = v2_of_angle(velocity_angle);
   };

   initialize_game();

   while (!glfwWindowShouldClose(window))
   {
      if (paused) glfwWaitEvents();
      else glfwPollEvents();

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
         break;

      i32 p_button_state = glfwGetKey(window, GLFW_KEY_P);
      if (p_button_last_state == GLFW_RELEASE &&  p_button_state == GLFW_PRESS)
         paused = !paused;
      p_button_last_state = p_button_state;

      if (paused)
         continue;

      f32 begin_time = glfwGetTime();

      {
         bool game_over = false;
         bool restart_requested = false;

         if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            started = true;
         if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            restart_requested = true;

         // TODO(hobrzut): Remove that.
         if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            ball_speed = 0.01f;
         else
            ball_speed = 1.6f;

         bg_time += delta_time;

         f32 player_velocity_x = 0.0f;
         if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player_velocity_x -= player_speed;
         if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player_velocity_x += player_speed;
         player_translate.x += delta_time * player_velocity_x;

         f32 max_left = -1.0f + player_body_half_width;
         f32 max_right = 1.0f - player_body_half_width;
         if (player_translate.x > max_right)
            player_translate.x = max_right;
         if (player_translate.x < max_left)
            player_translate.x = max_left;

         if (started)
         {
            v2 new_ball_translate = ball_translate + delta_time * ball_speed * ball_velocity;
            bool ball_disturbed = false;

            if (new_ball_translate.x < -1.0f || new_ball_translate.x > 1.0f)
            {
               ball_velocity.x = -ball_velocity.x;
               ball_disturbed = true;
            }
            if (new_ball_translate.y > 1.0f)
            {
               ball_velocity.y = -ball_velocity.y;
               ball_disturbed = true;
            }
            if (new_ball_translate.y < -1.1f - ball_half_radius)
               game_over = true;

            for (i32 i = 0; i < num_blocks; ++i)
            {
               v2 ball_block_diff = new_ball_translate - block_translations[i];
               f32 abs_diff_x = abs(ball_block_diff.x);
               f32 abs_diff_y = abs(ball_block_diff.y);
               f32 extent_x = block_half_width + ball_half_radius;
               f32 extent_y = block_half_height + ball_half_radius;

               if (abs_diff_x <= extent_x && abs_diff_y <= extent_y)
               {
                  f32 scaled_x = abs_diff_x / (block_half_width + ball_half_radius);
                  f32 scaled_y = abs_diff_y / (block_half_height + ball_half_radius);

                  if (scaled_x < scaled_y)
                     ball_velocity.y = -ball_velocity.y;
                  else
                     ball_velocity.x = -ball_velocity.x;

                  ball_disturbed = true;

                  swap(block_translations[i], block_translations[--num_blocks]);

                  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, board_vbo));
                  GL_CALL(glBufferData(GL_ARRAY_BUFFER, num_blocks * sizeof(v2), block_translations, GL_STATIC_DRAW));

                  break;
               }
            }

            if (ball_translate.y >= player_translate.y)
            {
               v2 ball_player_diff = new_ball_translate - player_translate;
               if (abs(ball_player_diff.x) <= player_body_half_width + ball_half_radius &&
                   abs(ball_player_diff.y) <= player_body_half_height + ball_half_radius)
               {
                  f32 bounce_x = ball_player_diff.x + player_body_half_width;

                  i32 segment_index = (i32)(bounce_x / player_segment_length);
                  if (segment_index < 0) segment_index = 0;
                  if (segment_index >= PLAYER_SEGMENTS) segment_index = PLAYER_SEGMENTS-1;

                  f32 bounce_angle = player_segment_bounce_angles[segment_index];
                  ball_velocity = v2_of_angle(bounce_angle);

                  ball_disturbed = true;
               }
            }

            if (!ball_disturbed)
               ball_translate = new_ball_translate;

            // Moving player could bump into the ball. In that case disconnect two bodies
            // by just teleporting the ball a little further.
            if (ball_translate.y >= player_translate.y)
            {
               v2 ball_player_diff = ball_translate - player_translate;
               if (abs(ball_player_diff.x) <= player_body_half_width + ball_half_radius &&
                   abs(ball_player_diff.y) <= player_body_half_height + ball_half_radius)
               {
                  v2 tv = ball_player_diff / ball_velocity;
                  assert(tv.x >= 0.0f && tv.y >= 0.0f);

                  f32 eps = 0.001f;
                  f32 t = min(tv.x, tv.y) + eps;
                  ball_translate += t * ball_velocity;
               }
            }
         }
         else
         {
            ball_follow_player();
         }

         GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

         // Draw background.
         GL_CALL(glUseProgram(bg_shader));
         GL_CALL(glBindVertexArray(bg_vao));
         GL_CALL(glUniform1f(bg_shader_time_uniform, bg_time));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw player.
         GL_CALL(glUseProgram(player_shader));
         GL_CALL(glBindVertexArray(player_vao));
         GL_CALL(glUniform2f(player_shader_scale_uniform, player_body_half_width, player_body_half_height));
         GL_CALL(glUniform2f(player_shader_translate_uniform, player_translate.x, player_translate.y));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw ball.
         GL_CALL(glUseProgram(ball_shader));
         GL_CALL(glBindVertexArray(ball_vao));
         GL_CALL(glUniform1f(ball_shader_radius_uniform, ball_half_radius));
         GL_CALL(glUniform2f(ball_shader_translate_uniform, ball_translate.x, ball_translate.y));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw blocks.
         GL_CALL(glUseProgram(block_shader));
         GL_CALL(glBindVertexArray(board_vao));
         GL_CALL(glUniform2f(block_shader_scale_uniform, block_half_width, block_half_height));
         GL_CALL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, num_blocks));

         glfwSwapBuffers(window);

         if (game_over || restart_requested) initialize_game();
      }

      delta_time = glfwGetTime() - begin_time;

      printf("\rFrame took %.3fms", delta_time * 1000);
   }

   return EXIT_SUCCESS;
}
