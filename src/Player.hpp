#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "Window.hpp"
#include "Common.hpp"
#include "Ball.hpp"

class Player
{
private:
    GLuint point_buffer;
    Line lines[5];

    glm::vec2 translate;

    void handleInput();
    void move(GLfloat dx);

public:
    void init();
    void draw();
    void update();

    void handleCollisions(Ball &ball);
};