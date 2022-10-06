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

// TODO(hoburzt): Make SOA out of it.
struct Vertex
{
   vec2 position;
   vec2 uv;
};

struct Rectangle
{
   vec2 position;
   float width;
   float height;
};

struct Player
{
   Rectangle body;
   float speed;

   GLuint vbo;
};

struct Ball
{
   vec2 translate;
   vec2 velocity;
   float speed;

   float radius;

   GLuint vbo;
};

struct Board
{
   int num_blocks;
   vec2 *block_positions; // left-bottom corner

   float block_width;
   float block_height;

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

void create_rectangle_vertices(Vertex vertices[4], float width, float height)
{
   vertices[0].position = vec2(-width*0.5f, -height*0.5f);
   vertices[1].position = vertices[0].position + vec2(width, 0.f);
   vertices[2].position = vertices[0].position + vec2(0.f, height);
   vertices[3].position = vertices[0].position + vec2(width, height);

   vertices[0].uv = vec2(-1.0f, -1.0f);
   vertices[1].uv = vec2(1.0f, -1.0f);
   vertices[2].uv = vec2(-1.0f, 1.0f);
   vertices[3].uv = vec2(1.0f, 1.0f);
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

vec2 reflect(vec2 v, Line line)
{
   vec2 w = line.p[0] - line.p[1];
   vec2 d = v - glm::dot(v, w) / glm::dot(w, w) * w;
   return v - 2.f * d;
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
   int size = std::min(width, height);
   int width_offset = (width - size) / 2;
   int height_offset = (height - size) / 2;

   glViewport(width_offset, height_offset, size, size);
}

void reset_game(Player *player, Ball *ball, Board *board)
{
   player->body.position = vec2(0.f, -0.94f);

   float velocity_angle = Random::sample(0.25f, 0.75f) * M_PI;
   ball->velocity = vec2(std::cos(velocity_angle), std::sin(velocity_angle));
   ball->translate = vec2(0.f, -0.85f);

   // TODO(hobrzut): Load from file instead.
   int n_rows = 8;
   int n_cols = 9;
   const char *board_text =
      "...XXX...\n"
      "...XXX...\n"
      "...XXX...\n"
      ".........\n"
      ".........\n"
      ".........\n"
      ".........\n"
      ".........\n";

   board->num_blocks = n_rows * n_cols;
   if (board->block_positions) free(board->block_positions);
   board->block_positions = (vec2 *)malloc(board->num_blocks * sizeof(vec2));
   vec2 *vertices = (vec2 *)malloc(board->num_blocks * 6 * sizeof(vec2));

   float screen_width = 2.0f;
   float screen_height = 2.0f;
   float border = 0.1f; // TODO(hobrzut): Customize.
   float padding = 0.02f; // TODO(hobrzut): Customize.
   float player_area = 0.2f; // TODO(hobrzut): Customize.

   float game_area_width = screen_width - 2*border - 2*padding;

   float between_blocks_padding = 0.01f; // TODO(hobrzut): Customize.

   float block_width = (game_area_width - (n_cols-1) * between_blocks_padding) / n_cols;
   float block_height = 0.05f; // TODO(hobrzut): Customize.

   board->block_width = block_width;
   board->block_height = block_height;

   float pos_y = screen_height - border - padding - block_height;
   for (int row = 0, vert_idx = 0, block_idx = 0; row < n_rows; ++row)
   {
      float pos_x = border + padding;
      for (int col = 0; col < n_cols; ++col, vert_idx += 6, ++block_idx)
      {
         vec2 p1 = vec2(pos_x, pos_y);
         vec2 p2 = p1 + vec2(block_width, 0.f);
         vec2 p3 = p1 + vec2(0.f, block_height);
         vec2 p4 = p1 + vec2(block_width, block_height);

         vertices[vert_idx + 0] = p1;
         vertices[vert_idx + 1] = p1;
         vertices[vert_idx + 2] = p3;
         vertices[vert_idx + 3] = p2;
         vertices[vert_idx + 4] = p4;
         vertices[vert_idx + 5] = p3;

         // TODO(hobrzut): Change to left-bottom corner.
         vec2 position = p1 + vec2(block_width*0.5f, block_height*0.5f);
         board->block_positions[block_idx] = p1;

         pos_x += between_blocks_padding + block_width;
      }

      pos_y += between_blocks_padding + block_height;
   }


   glBindBuffer(GL_ARRAY_BUFFER, board->vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * 6 * board->num_blocks, board->block_positions, GL_STATIC_DRAW);

   free(vertices);
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
      window_resize_handler(window, width, height);
   }

   if (!window)
   {
      fprintf(stderr, "Failed to open GLFW window.\n");
      return EXIT_FAILURE;
   }

   glfwMakeContextCurrent(window);
   glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
   glClearColor(7.f/255, 30.f/255, 34.f/255, 1.f);
   glfwSetFramebufferSizeCallback(window, window_resize_handler);

   if (glewInit() != GLEW_OK)
   {
      fprintf(stderr, "Failed to initialize GLEW.\n");
      return EXIT_FAILURE;
   }

   GLuint bg_shader = Shader::load(
         "../shader/background_vertex.glsl",
         "../shader/background_fragment.glsl");

   if (!bg_shader)
   {
      fprintf(stderr, "Failed to load background shader.\n");
      return EXIT_FAILURE;
   }

   GLuint player_shader = Shader::load(
         "../shader/player_vertex.glsl",
         "../shader/player_fragment.glsl");

   if (!player_shader)
   {
      fprintf(stderr, "Failed to load player shader.\n");
      return EXIT_FAILURE;
   }

   GLuint ball_shader = Shader::load(
         "../shader/ball_vertex.glsl",
         "../shader/ball_fragment.glsl");

   if (!ball_shader)
   {
      fprintf(stderr, "Failed to load ball shader.\n");
      return EXIT_FAILURE;
   }

   GLuint block_shader = Shader::load(
         "../shader/block_vertex.glsl",
         "../shader/block_fragment.glsl");

   if (!block_shader)
   {
      fprintf(stderr, "Failed to load block shader.\n");
      return EXIT_FAILURE;
   }

   GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time");
   GLint player_shader_translate_uniform = glGetUniformLocation(player_shader, "translate");
   GLint player_shader_color_uniform = glGetUniformLocation(player_shader, "color");
   GLint ball_shader_translate_uniform = glGetUniformLocation(ball_shader, "translate");
   GLint block_shader_translate_uniform = glGetUniformLocation(block_shader, "translate");

   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   GLuint bg_vbo;
   {
      vec2 screen_corners[] = {
         { -1.0f,  1.0f },
         { -1.0f, -1.0f },
         {  1.0f,  1.0f },
         {  1.0f, -1.0f }
      };

      glGenBuffers(1, &bg_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(screen_corners), screen_corners, GL_STATIC_DRAW);
   }

   Player player = {};
   {
      float width = 0.1f;
      float height = 0.01f;

      player.body.width = width;
      player.body.height = height;
      player.speed = 1.1f;

      Vertex vertices[4];
      create_rectangle_vertices(vertices, width, height);

      glGenBuffers(1, &player.vbo);
      glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      // TODO(hobrzut): Add enable/pointer here?
   }

   Ball ball = {};
   {
      float radius = 0.05f;

      ball.speed = 1.3f;
      ball.radius = radius;

      Vertex vertices[4];
      create_rectangle_vertices(vertices, radius, radius);

      glGenBuffers(1, &ball.vbo);
      glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   }

   Board board = {};
   glGenBuffers(1, &board.vbo);

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
      bool restart_requested = false;
      bool level_completed = false;

      if (!paused)
      {
         if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            started = true;
         if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            restart_requested = true;

         /* Simulate. */
         bg_time += delta_time;

         float player_move_x = 0.0f;
         if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player_move_x -= delta_time * player.speed;
         if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player_move_x += delta_time * player.speed;
         player.body.position.x = std::max(-0.9f, std::min(0.9f, player.body.position.x + player_move_x));

         if (started)
         {
            vec2 ball_new_translate = ball.translate + delta_time * ball.speed * ball.velocity;
            bool ball_disturbed = false;

            if (ball_new_translate.x >= 1.f || ball_new_translate.x <= -1.f)
            {
               ball.velocity.x = -ball.velocity.x;
               ball_disturbed = true;
            }
            if (ball_new_translate.y >= 1.f)
            {
               ball.velocity.y = -ball.velocity.y;
               ball_disturbed = true;
            }
            if (ball.translate.y < -1.f)
               game_over = true;

            for (int i = 0; i < board.num_blocks; ++i)
            {
               vec2 block_position = board.block_positions[i];
               float block_width = board.block_width;
               float block_height = board.block_height;

               if (ball_new_translate.x >= block_position.x - ball.radius &&
                   ball_new_translate.x <= block_position.x + block_width + ball.radius &&
                   ball_new_translate.y >= block_position.y - ball.radius &&
                   ball_new_translate.y <= block_position.y + block_height + ball.radius)
               {
               }
            }

            // for (int i = 0; i < board.num_triangles; ++i)
            // {
               // Line intersection_line;
               // if (is_intersection(ball_translated_triangle, board.triangles[i], &intersection_line))
               // {
                  // ball.velocity = reflect(ball.velocity, intersection_line);
                  // ball_disturbed = true;
//
                  // std::swap(board.triangles[i], board.triangles[--board.num_triangles]);
//
                  // glBindBuffer(GL_ARRAY_BUFFER, board.vbo);
                  // glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle) * board.num_triangles, board.triangles, GL_STATIC_DRAW);
//
                  // if (board.num_triangles == 0)
                     // level_completed = true;
//
                  // break;
               // }
            // }
//
            // for (int i = 0; i < 5; ++i)
            // {
               // vec2 p1 = player.body_points[i+1] + player.translate;
               // vec2 p2 = player.body_points[i+2] + player.translate;
               // Line line = create_line(p1, p2);
//
               // if (is_intersection(line, ball_translated_triangle))
               // {
                  // ball.velocity = reflect(ball.velocity, line);
                  // ball_disturbed = true;
//
                  // break;
               // }
            // }

            if (!ball_disturbed)
               ball.translate = ball_new_translate;
         }
         else
            ball.translate.x = player.body.position.x;


         glClear(GL_COLOR_BUFFER_BIT);

         /* Draw background. */
         glUseProgram(bg_shader);
         glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

         glUniform1f(bg_shader_time_uniform, bg_time);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

         glDisableVertexAttribArray(0);
         glUseProgram(0);

         /* Draw player. */
         // glUseProgram(player_shader);
         // glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
         // glEnableVertexAttribArray(0);
         // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
         // glEnableVertexAttribArray(1);
         // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, uv));
//
         // glUniform2f(player_shader_translate_uniform, player.body.position.x + 0.02f, player.body.position.y - 0.015f);
         // glUniform3f(player_shader_color_uniform, 0.f, 0.f, 0.f);
         // glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
//
         // glUniform2f(player_shader_translate_uniform, player.body.position.x, player.body.position.y);
         // glUniform3f(player_shader_color_uniform,  80.f/255, 120.f/255, 111.f/255);
         // glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
//
         // glUniform3f(player_shader_color_uniform,  24.f/255, 100.f/255, 97.f/255);
         // glDrawArrays(GL_TRIANGLE_FAN, 7, 7);
//
         // glDisableVertexAttribArray(0);
         // glDisableVertexAttribArray(1);

         /* Draw ball. */
         // glBindBuffer(GL_ARRAY_BUFFER, ball.vbo);
         // glEnableVertexAttribArray(0);
         // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
//
         // glUniform2f(standard_shader_translate_uniform, ball.translate.x + 0.01f, ball.translate.y - 0.01f);
         // glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         // glDrawArrays(GL_TRIANGLES, 0, 3);
//
         // glUniform2f(standard_shader_translate_uniform, ball.translate.x, ball.translate.y);
         // glUniform3f(standard_shader_color_uniform,  244.f/255, 192.f/255, 149.f/255);
         // glDrawArrays(GL_TRIANGLES, 0, 3);
//
         // // TODO(hobrzut): Maybe call enable/disable only once.
         // glDisableVertexAttribArray(0);
//
         // [> Draw board. <]
         // glBindBuffer(GL_ARRAY_BUFFER, board.vbo);
         // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
         // glEnableVertexAttribArray(0);
//
         // glUniform2f(standard_shader_translate_uniform, 0.01f, -0.01f);
         // glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         // glDrawArrays(GL_TRIANGLES, 0, 3*board.num_triangles);
//
         // glUniform2f(standard_shader_translate_uniform, 0.0f, 0.0f);
         // glUniform3f(standard_shader_color_uniform,  244.f/255, 192.f/255, 149.f/255);
         // glDrawArrays(GL_TRIANGLES, 0, 3*board.num_triangles);
//
         // glDisableVertexAttribArray(0);
//
         glfwSwapBuffers(window);

         if (game_over || restart_requested || level_completed)
         {
            reset_game(&player, &ball, &board);
            started = false;
         }

         delta_time = glfwGetTime() - begin_time;
      }
   }

   return EXIT_SUCCESS;
}
