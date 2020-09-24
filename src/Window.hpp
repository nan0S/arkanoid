#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Common.hpp"
#include "Shader.hpp"

#include <string>
#include <iostream>

class Window
{
public:
    static GLFWwindow *window;
    static int width, height;

    static bool create(int width, int height, std::string title);
    static bool windowShouldClose();

    static void windowResizeHandler(GLFWwindow *wind, int w, int h);

    static bool keyPressed(GLuint key);
    static bool buttonPressed(GLuint button);

    static void clear();
    static void display();
};