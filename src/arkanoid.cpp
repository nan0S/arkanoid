#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "error.h"

using v2 = glm::vec2;

#define min(a, b) a < b ? a : b;

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

void
create_rectangle_with_uv(v2 data[8], float half_width, float half_height)
{
   data[1] = {  half_width, -half_height };
   data[0] = { -half_width, -half_height };
   data[2] = { -half_width,  half_height };
   data[3] = {  half_width,  half_height };

   data[4] = { -1.0f, -1.0f };
   data[5] = {  1.0f, -1.0f };
   data[6] = { -1.0f,  1.0f };
   data[7] = {  1.0f,  1.0f };
}

inline float
random_unilateral()
{
   return (float)rand() / RAND_MAX;
}

inline float
random_between(float min, float max)
{
   float t = random_unilateral();
   return (1-t) * min + t * max;
}

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

int
main()
{
   if (!glfwInit())
   {
      fprintf(stderr, "Failed to initialize GLFW\n");
      return EXIT_FAILURE;
   }

   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow *window;
   {
      int width = 1080;
      int height = 1080;
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

      GL_CALL(glClearColor(7.f/255, 30.f/255, 34.f/255, 1.f));
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

   GL_CALL(GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time"));
   GL_CALL(GLint player_shader_translate_uniform = glGetUniformLocation(player_shader, "translate"));
   GL_CALL(GLint ball_shader_translate_uniform = glGetUniformLocation(ball_shader, "translate"));

   GLuint bg_vao;
   {
      GL_CALL(glGenVertexArrays(1, &bg_vao));
      GL_CALL(glBindVertexArray(bg_vao));

      v2 screen_corners[] = {
         { -1.0f, -1.0f },
         { -1.0f,  1.0f },
         {  1.0f, -1.0f },
         {  1.0f,  1.0f },
      };

      GLuint bg_vbo;
      GL_CALL(glGenBuffers(1, &bg_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bg_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(screen_corners), screen_corners, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
   }

   GLuint player_vao;
   v2 player_translate;
   float player_speed = 2.0f;
   float player_body_width = 0.2f;
   float player_body_height = 0.035f;
   float player_half_body_width = 0.5f * player_body_width;
   float player_half_body_height = 0.5f * player_body_height;
   const int PLAYER_SEGMENTS = 6;
   float player_segment_length = player_body_width / PLAYER_SEGMENTS;
   float player_segment_bounce_angles[PLAYER_SEGMENTS] = { 140, 115, 100, 80, 65, 40 };
   {
      glGenVertexArrays(1, &player_vao);
      glBindVertexArray(player_vao);

      v2 body_data[8];
      create_rectangle_with_uv(body_data, player_half_body_width, player_half_body_height);

      GLuint player_vbo;
      GL_CALL(glGenBuffers(1, &player_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, player_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(body_data), body_data, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
      GL_CALL(glEnableVertexAttribArray(1));
      GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void *)(4*sizeof(v2))));

      for (int i = 0; i < PLAYER_SEGMENTS; ++i)
         // TODO(hobrzut): Change M_PI to pi32.
         player_segment_bounce_angles[i] *= M_PI / 180;
   }

   GLuint ball_vao;
   float ball_radius = 0.025f;
   float ball_half_radius = 0.5f * ball_radius;
   v2 ball_translate;
   float ball_speed = 2.0f;
   v2 ball_velocity;
   {
      // TODO(hobrzut): Maybe remove duplication in the future.
      GL_CALL(glGenVertexArrays(1, &ball_vao));
      GL_CALL(glBindVertexArray(ball_vao));

      v2 body_data[8];
      create_rectangle_with_uv(body_data, ball_half_radius, ball_half_radius);

      GLuint vbo;
      GL_CALL(glGenBuffers(1, &vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(body_data), body_data, GL_STATIC_DRAW));

      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
      GL_CALL(glEnableVertexAttribArray(1));
      GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void *)(4*sizeof(v2))));
   }

   float bg_time = 0.0f;
   float delta_time = 0.0f;
   bool paused = false;
   int p_button_last_state = GLFW_RELEASE;
   bool started;

   auto initialize_game = [&started,
        &player_translate,
        &ball_translate,
        &ball_velocity,
        ball_half_radius,
        player_half_body_height]()
   {
      started = false;

      player_translate = { 0.0f, -0.86f };

      float eps = 0.001f;
      ball_translate.x = player_translate.x;
      ball_translate.y = player_translate.y + player_half_body_height + ball_half_radius + eps;

      float velocity_angle = random_between(M_PI/4, 3*M_PI/4);
      ball_velocity = v2_from_angle(velocity_angle);
   };

   initialize_game();

   while (!glfwWindowShouldClose(window))
   {
      if (paused) glfwWaitEvents();
      else glfwPollEvents();

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
         break;

      int p_button_state = glfwGetKey(window, GLFW_KEY_P);
      if (p_button_last_state == GLFW_RELEASE &&  p_button_state == GLFW_PRESS)
         paused = !paused;
      p_button_last_state = p_button_state;

      if (paused)
         continue;

      float begin_time = glfwGetTime();

      {
         bool game_over = false;
         bool restart_requested = false;

         if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            started = true;
         if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            restart_requested = true;

         if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            ball_speed = 2.0f;
         else
            ball_speed = 0.01f;

         bg_time += delta_time;

         float player_velocity_x = 0.0f;
         if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player_velocity_x -= player_speed;
         if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player_velocity_x += player_speed;
         player_translate.x += delta_time * player_velocity_x;

         float max_left = -1.0f + player_half_body_width;
         float max_right = 1.0f - player_half_body_width;
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
            if (new_ball_translate.y < -1.0f)
               game_over = true;

            if (ball_translate.y >= player_translate.y)
            {
               v2 ball_player_diff = new_ball_translate - player_translate;
               if (abs(ball_player_diff.x) <= player_half_body_width + ball_half_radius &&
                   abs(ball_player_diff.y) <= player_half_body_height + ball_half_radius)
               {
                  float bounce_x = ball_player_diff.x + player_half_body_width;
                  int segment_index = (int)(bounce_x / player_segment_length);
                  if (segment_index < 0) segment_index = 0;
                  if (segment_index >= PLAYER_SEGMENTS) segment_index = PLAYER_SEGMENTS-1;

                  float bounce_angle = player_segment_bounce_angles[segment_index];
                  ball_velocity = v2_from_angle(bounce_angle);

                  ball_disturbed = true;
               }
            }

            if (!ball_disturbed)
               ball_translate = new_ball_translate;

            if (ball_translate.y >= player_translate.y)
            {
               v2 ball_player_diff = ball_translate - player_translate;
               if (abs(ball_player_diff.x) <= player_half_body_width + ball_half_radius &&
                   abs(ball_player_diff.y) <= player_half_body_height + ball_half_radius)
               {
                  v2 tv = ball_player_diff / ball_velocity;
                  assert(tv.x >= 0.0f && tv.y >= 0.0f);

                  float eps = 0.001f;
                  float t = min(tv.x, tv.y) + eps;
                  ball_translate += t * ball_velocity;
               }
            }
         }
         else
         {
            // TODO(hobrzut): Extract into common function ([initialize_game]).
            float eps = 0.001f;
            ball_translate.x = player_translate.x;
            ball_translate.y = player_translate.y + player_half_body_height + ball_half_radius + eps;
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
         GL_CALL(glUniform2f(player_shader_translate_uniform, player_translate.x, player_translate.y));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         // Draw ball.
         GL_CALL(glUseProgram(ball_shader));
         GL_CALL(glBindVertexArray(ball_vao));
         GL_CALL(glUniform2f(ball_shader_translate_uniform, ball_translate.x, ball_translate.y));
         GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

         glfwSwapBuffers(window);

         if (game_over || restart_requested) initialize_game();
      }

      delta_time = glfwGetTime() - begin_time;

      printf("Frame took %.3fms\n", delta_time * 1000);
   }

   return EXIT_SUCCESS;
}
