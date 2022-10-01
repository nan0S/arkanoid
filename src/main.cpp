#include "Game.hpp"

int main2();

int main()
{
   // return main2();
   Game game;
   game.start();

   return 0;
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
   glm::vec2 screen_corners[] = {
      { -1.0f, 1.0f },
      { -1.0f, -1.0f },
      { 1.0f, 1.0f },
      { 1.0f, -1.0f }
   };

   GLuint bg_vbo;
   glGenBuffers(1, &bg_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(screen_corners), screen_corners, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

   glm::vec2 player_points[] = {
      { 0.0f, 0.0f },
      { -0.1f, 0.00f },
      { -0.06f, 0.04f },
      { -0.03f, 0.05f },
      { 0.03f, 0.05f },
      { 0.06f, 0.04f },
      { 0.1f, 0.0f }
   };

   // TODO(hobrzut): Subscope.
   GLuint player_vbo;
   glGenBuffers(1, &player_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, player_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(player_points), player_points, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

   GLint standard_shader_translate_uniform = glGetUniformLocation(standard_shader, "translate");
   GLint standard_shader_color_uniform = glGetUniformLocation(standard_shader, "color");

   bool paused = false;
   int p_last_state = GLFW_RELEASE;
   float bg_time = 0;
   glm::vec2 player_translate = {};

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
         glClear(GL_COLOR_BUFFER_BIT);

         /* Draw background. */
         glUseProgram(bg_shader);
         glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

         glUniform1f(bg_shader_time_uniform, bg_time);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

         glDisableVertexAttribArray(0);

         /* Draw player. */
         glUseProgram(standard_shader);
         glBindBuffer(GL_ARRAY_BUFFER, player_vbo);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

         glUniform2f(standard_shader_translate_uniform, player_translate.x + 0.02f, player_translate.y - 0.015f);
         glUniform3f(standard_shader_color_uniform, 0.f, 0.f, 0.f);
         glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

         glUniform2f(standard_shader_translate_uniform, player_translate.x, player_translate.y);
         glUniform3f(standard_shader_color_uniform,  80.f/255, 120.f/255, 111.f/255);
         glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

         glUniform3f(standard_shader_color_uniform,  24.f/255, 100.f/255, 97.f/255);
         glDrawArrays(GL_TRIANGLE_FAN, 7, 7);

         glDisableVertexAttribArray(0);

         glfwSwapBuffers(window);

         bg_time += glfwGetTime() - begin_time;
      }
   }

   // GLuint game_shader = Load
   // initGame();
   // run();
   // GLuint game_shader = Load
   // initGame();
   // run();

   return EXIT_SUCCESS;
}
