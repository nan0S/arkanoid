#include "Ball.hpp"

void Ball::init()
{
    translate = glm::vec2(0.0f, -0.8f);
    velocity = Math::normalized(glm::vec2(Random::randnorm(), Random::random(0.5f, 1.0f)));
    last_time = (GLfloat)glfwGetTime();

    triangle = Triangle(glm::vec2(0.0f, 0.0f), 0.04f, false);

    glGenBuffers(1, &point_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle), &triangle, GL_STATIC_DRAW);

    initVelocity();
}

void Ball::initVelocity()
{
    vel_points = new glm::vec2[2]{{0, 0}, velocity};
    glGenBuffers(1, &vel_point_buffer);
}

void Ball::draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform2f(0, translate.x + 0.01f, translate.y - 0.01f);
    glUniform3f(1, Color::BLACK.r, Color::BLACK.g, Color::BLACK.b);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUniform2f(0, translate.x, translate.y);
    glUniform3f(1, Color::BALL.r, Color::BALL.g, Color::BALL.b);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glDisableVertexAttribArray(0);

    // drawVelocity();
}

void Ball::drawVelocity()
{
    vel_points[1] = velocity / 10.0f;

    glBindBuffer(GL_ARRAY_BUFFER, vel_point_buffer);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), vel_points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform2f(0, translate.x, translate.y);
    glUniform3f(1, Color::YELLOW.r, Color::YELLOW.g, Color::YELLOW.b);

    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Ball::update()
{
    GLfloat now = (GLfloat)glfwGetTime();
    GLfloat delta_time = (GLfloat)now - last_time;
    last_time = now;
    translate += speed * delta_time * velocity;
}

Triangle Ball::getTriangle()
{
    return triangle + translate;  
}

void Ball::bounce(Line l, bool along_normal)
{
    glm::vec2 w = l.p1 - l.p2; 
    glm::vec2 d = velocity - Math::cross(velocity, w) / Math::cross(w, w) * w;
    if (along_normal)
        velocity = Math::normalized(-d);
    else
        velocity -= 2.0f * d;
}

void Ball::shake()
{
    velocity.x += Random::random(-0.1f, 0.1f);
    velocity = Math::normalized(velocity);
}

