#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "error.h"

using vec2 = glm::vec2;

void window_resize_handler(GLFWwindow *, int width, int height)
{
   int size = std::min(width, height);
   int width_offset = (width - size) / 2;
   int height_offset = (height - size) / 2;

   glViewport(width_offset, height_offset, size, size);
}

void glfw_error_callback(int code, const char* desc)
{
   fprintf(stderr, "GLFW error [%d]: %s\n", code, desc);
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

      if (!window)
      {
         fprintf(stderr, "Failed to open GLFW window.\n");
         return EXIT_FAILURE;
      }

      window_resize_handler(window, width, height);

      glfwMakeContextCurrent(window);
      glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
      glfwSetFramebufferSizeCallback(window, window_resize_handler);
      glfwSetErrorCallback(glfw_error_callback);

      GL_CALL(glClearColor(7.f/255, 30.f/255, 34.f/255, 1.f));
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

   GL_CALL(GLint bg_shader_time_uniform = glGetUniformLocation(bg_shader, "time"));

   GLuint vao;
   GL_CALL(glGenVertexArrays(1, &vao));
   GL_CALL(glBindVertexArray(vao));

   GLuint bg_vbo;
   {
      vec2 screen_corners[] = {
         { -1.0f,  1.0f },
         { -1.0f, -1.0f },
         {  1.0f,  1.0f },
         {  1.0f, -1.0f },
      };

      GL_CALL(glGenBuffers(1, &bg_vbo));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bg_vbo));
      GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(screen_corners), screen_corners, GL_STATIC_DRAW));
   }

   float bg_time = 0.f;
   float delta_time = 0.f;

   while (!glfwWindowShouldClose(window))
   {
      glfwPollEvents();

      if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
         break;

      float begin_time = glfwGetTime();
      bg_time += delta_time;

      GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

      /* Draw background. */
      GL_CALL(glUseProgram(bg_shader));
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bg_vbo));
      GL_CALL(glEnableVertexAttribArray(0));
      GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));

      GL_CALL(glUniform1f(bg_shader_time_uniform, bg_time));
      GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

      GL_CALL(glDisableVertexAttribArray(0));
      GL_CALL(glUseProgram(0));
      glfwSwapBuffers(window);

      delta_time = glfwGetTime() - begin_time;
   }

   return EXIT_SUCCESS;
}
