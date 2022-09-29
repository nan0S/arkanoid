#include "Game.hpp"

int main2();

int main()
{
   return main2();
   // Game game;
   // game.start();
//
   // return 0;
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

   GLint time_uniform = glGetUniformLocation(bg_shader, "time");

   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   glm::vec2 corners[] =
   {
      glm::vec2(-1.0f, 1.0f),
      glm::vec2(-1.0f, -1.0f),
      glm::vec2(1.0f, 1.0f),
      glm::vec2(1.0f, -1.0f)
   };

   GLuint bg_vbo;
   glGenBuffers(1, &bg_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

   while (!glfwWindowShouldClose(window))
   {
      float now = glfwGetTime();

      glfwPollEvents();
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
         break;

      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(bg_shader);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glUniform1f(time_uniform, now);
      glfwSwapBuffers(window);
   }

   // GLuint game_shader = Load
   // initGame();
   // run();
   // GLuint game_shader = Load
   // initGame();
   // run();

   return EXIT_SUCCESS;
}
