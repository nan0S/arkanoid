// #include "Game.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdio.h>
#include <random>

#include "Shader.hpp"

using vec2 = glm::vec2;
using vec3 = glm::vec3;

int main2();

int main()
{
   return main2();
   // Game game;
   // game.start();
//
   // return 0;
}

// TODO(hobrzut): Move to another file.
struct Line
{
   vec2 p[2];
};

Line create_line(vec2 a, vec2 b)
{
   Line line;
   line.p[0] = a;
   line.p[1] = b;
   return line;
}

struct Triangle
{
   vec2 p[3];
};

Triangle create_equaliteral_triangle(vec2 center, float side_length, bool inverted)
{
   Triangle t;
   // TODO(hobrzut): Cache sqrt(3)/4.
   float height = side_length * sqrtf(3.f) / 6.f;
   if (inverted)
   {
       t.p[0] = vec2(center.x - 0.5f*side_length, center.y + height);
       t.p[1] = vec2(center.x + 0.5f*side_length, center.y + height);
       t.p[2] = vec2(center.x, center.y - 2.f*height);
   }
   else
   {
       t.p[0] = vec2(center.x - 0.5f*side_length, center.y - height);
       t.p[1] = vec2(center.x, center.y + 2.f*height);
       t.p[2] = vec2(center.x + 0.5f*side_length, center.y - height);
   }

   return t;
}

struct Player
{
   float speed;
   vec2 translate;
   GLuint vbo;
};

struct Ball
{
   float speed;
   vec2 velocity;
   vec2 translate;
   GLuint vbo;
};

namespace Random
{
   std::mt19937 gen{std::random_device{}()};

   template<typename T>
   T sample(T min, T max)
   {
       using dist = std::conditional_t<
           std::is_integral<T>::value,
           std::uniform_int_distribution<T>,
           std::uniform_real_distribution<T>
       >;
       return dist{min, max}(gen);
   }
}

void window_resize_handler(GLFWwindow *, int width, int height)
{
   glViewport(0, 0, width, height);
}

int main2()
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

   int width = 1080;
   int height = 1080;
   GLFWwindow *window = glfwCreateWindow(width, height, "Arkanoid", 0, 0);
   glViewport(0, 0, width, height);

   if (!window)
   {
      fprintf(stderr, "Failed to open GLFW window.\n");
      return EXIT_FAILURE;
   }

   glfwMakeContextCurrent(window);
   // TODO(hobrzut): Check what that does.
   // glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
   glClearColor(7.f/255, 30.f/255, 34.f/255, 1.f);
   glfwSetFramebufferSizeCallback(window, window_resize_handler);

   // TODO(hobrzut): Check what that does.
   glewExperimental = 1;
   if (glewInit() != GLEW_OK)
   {
      fprintf(stderr, "Failed to initialize GLEW\n");
      return EXIT_FAILURE;
   }

   GLuint standard_shader = Shader::load("../shader/standardVertexShader.glsl",
                                         "../shader/standardFragmentShader.glsl");
   if (!standard_shader)
   {
      fprintf(stderr, "Failed to load standard shader.\n");
      return EXIT_FAILURE;
   }
   GLuint bg_shader = Shader::load("../shader/bgVertexShader.glsl",
                                   "../shader/bgFragmentShader.glsl");
   if (!bg_shader)
   {
      fprintf(stderr, "Failed to load background shader.\n");
      return EXIT_FAILURE;
   }

   GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time");

   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   // TODO(hobrzut): Subscope.
   vec2 screen_corners[] = {
      { -1.0f, 1.0f },
      { -1.0f, -1.0f },
      { 1.0f, 1.0f },
      { 1.0f, -1.0f }
   };

   // TODO(hobrzut): Batch all triangles.
   GLuint bg_vbo;
   glGenBuffers(1, &bg_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(screen_corners), screen_corners, GL_STATIC_DRAW);

   Player player = {};
   player.speed = 1.1f;
   player.translate = vec2(0.f, -0.94f);

   vec2 player_points[] = {
      { 0.0f, 0.0f },
      { -0.1f, 0.00f },
      { -0.06f, 0.04f },
      { -0.03f, 0.05f },
      { 0.03f, 0.05f },
      { 0.06f, 0.04f },
      { 0.1f, 0.0f }
   };

   // TODO(hobrzut): Move [glGenBuffers] into one call.
   glGenBuffers(1, &player.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(player_points), player_points, GL_STATIC_DRAW);

   Ball ball = {};
   ball.speed = 1.3f;
   float velocity_angle = Random::sample(0.25f, 0.75f) * M_PI;
   ball.velocity = vec2(std::cos(velocity_angle), std::sin(velocity_angle));
   ball.translate = vec2(0.f, 0.8f);

   Triangle ball_triangle = create_equaliteral_triangle(vec2(0.f), 0.04f, false);
   glGenBuffers(1, &ball.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(ball_triangle.p), ball_triangle.p, GL_STATIC_DRAW);

   int board_rows = 3;
   int board_cols = 4;
   int num_board_triangles = 2*board_rows*board_cols;
   Triangle *board_triangles = (Triangle *)malloc(num_board_triangles * sizeof(Triangle));
   float triangle_side_length = 0.11f;
   float triangle_height = sqrtf(3.f) / 6.f * triangle_side_length;
   vec2 global_offset(-(board_cols + 0.5f) * triangle_side_length * 0.5f, 0.65f);

   for (int i = 0; i < board_rows; ++i)
      for (int j = 0; j < board_cols; ++j)
      {
         int idx = 2 * (i*board_cols + j);
         vec2 shared_offset(j*triangle_side_length, -i*triangle_height*3.f - 3.f*triangle_height*(i-1));

         float side_length = triangle_side_length * 0.8f;

         vec2 local_offset1(triangle_side_length*0.5f, triangle_height);
         vec2 triangle1_center = global_offset + shared_offset + local_offset1;
         board_triangles[idx] = create_equaliteral_triangle(triangle1_center, side_length, false);

         vec2 local_offset2(triangle_side_length, 2.f*triangle_height);
         vec2 triangle2_center = global_offset + shared_offset + local_offset2;
         board_triangles[idx+1] = create_equaliteral_triangle(triangle2_center, side_length, true);
      }

   GLuint board_vbo;
   glGenBuffers(1, &board_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, board_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle) * num_board_triangles, board_triangles, GL_STATIC_DRAW);

   GLint standard_shader_translate_uniform = glGetUniformLocation(standard_shader, "translate");
   GLint standard_shader_color_uniform = glGetUniformLocation(standard_shader, "color");

   bool paused = false;
   int p_last_state = GLFW_RELEASE;
   float bg_time = 0;
   float delta_time = 0.f;

   while (!glfwWindowShouldClose(window))
   {
      if (paused)
         glfwWaitEvents();
      else
         glfwPollEvents();

      float begin_time = glfwGetTime();

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
         break;

      int p_state = glfwGetKey(window, GLFW_KEY_P);
      if (p_state == GLFW_PRESS && p_last_state == GLFW_RELEASE)
         paused = !paused;
      p_last_state = p_state;

      if (!paused)
      {
         /* Simulate. */
         bg_time += delta_time;

         if (glfwGetKey(window, GLFW_KEY_A))
            player.translate.x -= delta_time * player.speed;
         if (glfwGetKey(window, GLFW_KEY_D))
            player.translate.x += delta_time * player.speed;
         player.translate.x = std::max(-0.9f, std::min(0.9f, player.translate.x));

         ball.translate += delta_time * ball.speed * ball.velocity;
         if (ball.translate.x >= 1.f || ball.translate.x <= -1.f)
            ball.velocity.x = -ball.velocity.x;
         if (ball.translate.y >= 1.f || ball.translate.y <= -1.f)
            ball.velocity.y = -ball.velocity.y;

         glClear(GL_COLOR_BUFFER_BIT);

         /* Draw background. */
         glUseProgram(bg_shader);
         glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

         glUniform1f(bg_shader_time_uniform, bg_time);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

         glDisableVertexAttribArray(0);

         glUseProgram(standard_shader);

         /* Draw player. */
         glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

         glUniform2f(standard_shader_translate_uniform, player.translate.x + 0.02f, player.translate.y - 0.015f);
         glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

         glUniform2f(standard_shader_translate_uniform, player.translate.x, player.translate.y);
         glUniform3f(standard_shader_color_uniform,  80.f/255, 120.f/255, 111.f/255);
         glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

         glUniform3f(standard_shader_color_uniform,  24.f/255, 100.f/255, 97.f/255);
         glDrawArrays(GL_TRIANGLE_FAN, 7, 7);

         glDisableVertexAttribArray(0);

         /* Draw ball. */
         glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

         glUniform2f(standard_shader_translate_uniform, ball.translate.x + 0.01f, ball.translate.y - 0.01f);
         glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         glDrawArrays(GL_TRIANGLES, 0, 3);

         glUniform2f(standard_shader_translate_uniform, ball.translate.x, ball.translate.y);
         glUniform3f(standard_shader_color_uniform,  244.f/255, 192.f/255, 149.f/255);
         glDrawArrays(GL_TRIANGLES, 0, 3);

         // TODO(hobrzut): Maybe call enable/disable only once.
         glDisableVertexAttribArray(0);

         /* Draw board. */
         glBindBuffer(GL_ARRAY_BUFFER, board_vbo);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(0);

         glUniform2f(standard_shader_translate_uniform, 0.01f, -0.01f);
         glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         glDrawArrays(GL_TRIANGLES, 0, 3*num_board_triangles);

         glUniform2f(standard_shader_translate_uniform, 0.0f, 0.0f);
         glUniform3f(standard_shader_color_uniform,  244.f/255, 192.f/255, 149.f/255);
         glDrawArrays(GL_TRIANGLES, 0, 3*num_board_triangles);

         glDisableVertexAttribArray(0);

         glfwSwapBuffers(window);

         delta_time = glfwGetTime() - begin_time;
      }
   }

   return EXIT_SUCCESS;
}
