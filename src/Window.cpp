#include "Window.hpp"

GLFWwindow* Window::window = nullptr;
int Window::width = 0;
int Window::height = 0;

bool Window::create(int w, int h, std::string title)
{
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    width = w; height = h;
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (window == NULL)
        return false;
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    Color bg = Color::BG;
    glClearColor(bg.r, bg.g, bg.b, 1.0f);

    glfwSetFramebufferSizeCallback(window, windowResizeHandler);

    return true;
}

void Window::windowResizeHandler(GLFWwindow*, int w, int h)
{
    width = w; height = h;
    glViewport(0, 0, w, h);
}

bool Window::windowShouldClose()
{
    return keyPressed(GLFW_KEY_ESCAPE) || glfwWindowShouldClose(window);
}

bool Window::keyPressed(GLuint key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::buttonPressed(GLuint button)
{
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void Window::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::display()
{
    glfwSwapBuffers(window); 
}
