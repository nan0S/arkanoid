// #include "Game.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdio.h>
#include <random>

#include "Shader.hpp"

using vec2 = glm::vec2;
using vec3 = glm::vec3;

// TODO(hobrzut): Move to another file.
struct Line
{
   vec2 p[2];
};

struct Triangle
{
   vec2 p[3]; // anti-clockwise order
};

struct Background
{
   GLuint vbo;
};

struct Player
{
   float speed;
   vec2 translate;
   GLuint vbo;
   vec2 body_points[7];
};

struct Ball
{
   float speed;
   vec2 velocity;
   vec2 translate;
   GLuint vbo;
   Triangle body;
};

struct Board
{
   int rows, cols;
   int num_triangles;
   Triangle *triangles;
   GLuint vbo;
};

float cross(vec2 u, vec2 v)
{
   return u.x*v.y - u.y*v.x;
}

Line create_line(vec2 a, vec2 b)
{
   Line line;
   line.p[0] = a;
   line.p[1] = b;
   return line;
}

bool binary_orientation(vec2 p, Line l)
{
   return cross(p-l.p[1], l.p[0]-l.p[1]) > 0;
}

Triangle create_equaliteral_triangle(vec2 center, float side_length, bool inverted)
{
   Triangle t;
   // TODO(hobrzut): Cache sqrt(3)/4.
   float height = side_length * sqrtf(3.f) / 6.f;
   if (inverted)
   {
       t.p[0] = vec2(center.x + 0.5f*side_length, center.y + height);
       t.p[1] = vec2(center.x - 0.5f*side_length, center.y + height);
       t.p[2] = vec2(center.x, center.y - 2.f*height);
   }
   else
   {
       t.p[0] = vec2(center.x - 0.5f*side_length, center.y - height);
       t.p[1] = vec2(center.x + 0.5f*side_length, center.y - height);
       t.p[2] = vec2(center.x, center.y + 2.f*height);
   }

   return t;
}

bool is_intersection(Line l1, Line l2)
{
   return binary_orientation(l1.p[0], l2) != binary_orientation(l1.p[1], l2)
       && binary_orientation(l2.p[0], l1) != binary_orientation(l2.p[1], l1);
}

bool is_intersection(Line l, Triangle t)
{
   Line lines[3] = {
      create_line(t.p[0], t.p[1]),
      create_line(t.p[1], t.p[2]),
      create_line(t.p[2], t.p[0])
   };

   for (int i = 0; i < 3; ++i)
      if (is_intersection(l, lines[i]))
         return true;

   return false;
}

// TODO(hobrzut): Change return value/name function convention (multiple lines).
bool is_intersection(Triangle t1, Triangle t2, Line *intersection_line)
{
   Line lines[] = {
      create_line(t2.p[0], t2.p[1]),
      create_line(t2.p[1], t2.p[2]),
      create_line(t2.p[2], t2.p[0])
   };

   for (int i = 0; i < 3; ++i)
      if (is_intersection(lines[i], t1))
      {
         *intersection_line = lines[i];
         return true;
      }

   return false;
}


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

void reset_game(Player *player, Ball *ball, Board *board)
{
   player->translate = vec2(0.f, -0.94f);

   float velocity_angle = Random::sample(0.25f, 0.75f) * M_PI;
   ball->velocity = vec2(std::cos(velocity_angle), std::sin(velocity_angle));
   ball->translate = vec2(0.f, -0.85f);

   board->num_triangles = 2*board->rows*board->cols;
   if (!board->triangles)
      board->triangles = (Triangle *)malloc(board->num_triangles * sizeof(Triangle));

   float triangle_side_length = 0.11f;
   float triangle_height = sqrtf(3.f) / 6.f * triangle_side_length;
   vec2 global_offset(-(board->cols + 0.5f) * triangle_side_length * 0.5f, 0.65f);

   for (int i = 0; i < board->rows; ++i)
      for (int j = 0; j < board->cols; ++j)
      {
         int idx = 2 * (i*board->cols + j);
         vec2 shared_offset(j*triangle_side_length, -i*triangle_height*3.f - 3.f*triangle_height*(i-1));

         float side_length = triangle_side_length * 0.8f;

         vec2 local_offset1(triangle_side_length*0.5f, triangle_height);
         vec2 triangle1_center = global_offset + shared_offset + local_offset1;
         board->triangles[idx] = create_equaliteral_triangle(triangle1_center, side_length, false);

         vec2 local_offset2(triangle_side_length, 2.f*triangle_height);
         vec2 triangle2_center = global_offset + shared_offset + local_offset2;
         board->triangles[idx+1] = create_equaliteral_triangle(triangle2_center, side_length, true);
      }

   glBindBuffer(GL_ARRAY_BUFFER, board->vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle) * board->num_triangles, board->triangles, GL_STATIC_DRAW);
}

int main()
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
      glViewport(0, 0, width, height);
   }

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

   GLint standard_shader_translate_uniform = glGetUniformLocation(standard_shader, "translate");
   GLint standard_shader_color_uniform = glGetUniformLocation(standard_shader, "color");
   GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time");

   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   Background bg = {};
   {
      vec2 screen_corners[] = {
         { -1.0f, 1.0f },
         { -1.0f, -1.0f },
         { 1.0f, 1.0f },
         { 1.0f, -1.0f }
      };

      // TODO(hobrzut): Batch all triangles.
      glGenBuffers(1, &bg.vbo);
      glBindBuffer(GL_ARRAY_BUFFER, bg.vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(screen_corners), screen_corners, GL_STATIC_DRAW);
   }

   Player player = {};
   {
      player.speed = 1.1f;
      player.body_points[0] = vec2(0.0f, 0.0f);
      player.body_points[1] = vec2(-0.1f, 0.00f);
      player.body_points[2] = vec2(-0.06f, 0.04f);
      player.body_points[3] = vec2(-0.03f, 0.05f);
      player.body_points[4] = vec2(0.03f, 0.05f);
      player.body_points[5] = vec2(0.06f, 0.04f);
      player.body_points[6] = vec2(0.1f, 0.0f);

      // TODO(hobrzut): Move [glGenBuffers] into one call.
      glGenBuffers(1, &player.vbo);
      glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(player.body_points), player.body_points, GL_STATIC_DRAW);
   }

   Ball ball = {};
   {
      ball.speed = 1.3f;
      ball.body = create_equaliteral_triangle(vec2(0.f), 0.04f, false);

      glGenBuffers(1, &ball.vbo);
      glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(ball.body.p), ball.body.p, GL_STATIC_DRAW);
   }

   Board board = {};
   {
      board.rows = 3;
      board.cols = 4;
      glGenBuffers(1, &board.vbo);
   }

   reset_game(&player, &ball, &board);

   bool paused = false;
   bool started = false;
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

      bool game_over = false;
      bool game_restart = false;

      if (!paused)
      {
         if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            started = true;
         if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            game_restart = true;

         /* Simulate. */
         bg_time += delta_time;

         if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player.translate.x -= delta_time * player.speed;
         if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player.translate.x += delta_time * player.speed;
         player.translate.x = std::max(-0.9f, std::min(0.9f, player.translate.x));

         if (started)
         {
            ball.translate += delta_time * ball.speed * ball.velocity;
            if (ball.translate.x >= 1.f || ball.translate.x <= -1.f)
               ball.velocity.x = -ball.velocity.x;
            if (ball.translate.y >= 1.f)
               ball.velocity.y = -ball.velocity.y;
            if (ball.translate.y < -1.f)
               game_over = true;

            Triangle ball_translated_triangle = ball.body;
            for (int i = 0; i < 3; ++i)
               ball_translated_triangle.p[i] += ball.translate;

            for (int i = 0; i < board.num_triangles; ++i)
            {
               Line intersection_line;
               if (is_intersection(ball_translated_triangle, board.triangles[i], &intersection_line))
               {
                  vec2 w = intersection_line.p[0] - intersection_line.p[1];
                  vec2 d = ball.velocity - glm::dot(ball.velocity, w) / glm::dot(w, w) * w;
                  ball.velocity -= 2.f * d;

                  std::swap(board.triangles[i], board.triangles[--board.num_triangles]);
                  glBindBuffer(GL_ARRAY_BUFFER, board.vbo);
                  glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle) * board.num_triangles, board.triangles, GL_STATIC_DRAW);

                  break;
               }
            }

            for (int i = 0; i < 5; ++i)
            {
               vec2 p1 = player.body_points[i+1] + player.translate;
               vec2 p2 = player.body_points[i+2] + player.translate;
               Line line = create_line(p1, p2);

               if (is_intersection(line, ball_translated_triangle))
               {
                  vec2 w = line.p[0] - line.p[1];
                  vec2 d = ball.velocity - glm::dot(ball.velocity, w) / glm::dot(w, w) * w;
                  ball.velocity -= 2.f * d;

                  break;
               }
            }
         }
         else
            ball.translate.x = player.translate.x;


         glClear(GL_COLOR_BUFFER_BIT);

         /* Draw background. */
         glUseProgram(bg_shader);
         glBindBuffer(GL_ARRAY_BUFFER, bg.vbo);
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
         glBindBuffer(GL_ARRAY_BUFFER, board.vbo);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(0);

         glUniform2f(standard_shader_translate_uniform, 0.01f, -0.01f);
         glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         glDrawArrays(GL_TRIANGLES, 0, 3*board.num_triangles);

         glUniform2f(standard_shader_translate_uniform, 0.0f, 0.0f);
         glUniform3f(standard_shader_color_uniform,  244.f/255, 192.f/255, 149.f/255);
         glDrawArrays(GL_TRIANGLES, 0, 3*board.num_triangles);

         glDisableVertexAttribArray(0);

         glfwSwapBuffers(window);

         if (game_over || game_restart)
         {
            reset_game(&player, &ball, &board);
            started = false;
         }

         delta_time = glfwGetTime() - begin_time;
      }
   }

   return EXIT_SUCCESS;
}
