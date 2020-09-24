#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "Triangle.hpp"

class Ball
{
private:
    GLuint point_buffer;

    Triangle triangle;
    glm::vec2 translate;

    const GLfloat speed = 1.3f;
    GLfloat last_time;

    GLuint vel_point_buffer;
    glm::vec2 *vel_points;

public:
    glm::vec2 velocity;
    
    Ball() {}

    void init();
    void draw();
    void update();
    void initVelocity();
    void drawVelocity();
    void bounce(Line l, bool along_normal = false);
    void shake();

    Triangle getTriangle();
};