#include "Player.hpp"

void Player::init()
{
    translate = glm::vec2(0.0f, -0.94f);

    glm::vec2 points[14] = {glm::vec2(0.0f, 0.0f),
                            glm::vec2(-0.1f, 0.00f),
                            glm::vec2(-0.06f, 0.04f),
                            glm::vec2(-0.03f, 0.05f),
                            glm::vec2(0.03f, 0.05f),
                            glm::vec2(0.06f, 0.04f),
                            glm::vec2(0.1f, 0.0f)};

    for (int i = 0; i < 7; ++i)
        points[7 + i] = points[i] * 0.8f;

    for (int i = 0; i < 5; ++i)
        lines[i] = Line(points[i + 1], points[i + 2]);

    glGenBuffers(1, &point_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
    glBufferData(GL_ARRAY_BUFFER, 14 * sizeof(glm::vec2), points, GL_STATIC_DRAW);
}

void Player::draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glUniform2f(0, translate.x + 0.02f, translate.y - 0.015f);
    glUniform3f(1, Color::BLACK.r, Color::BLACK.g, Color::BLACK.b);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

    glUniform2f(0, translate.x, translate.y);
    glUniform3f(1, Color::PLAYER2.r, Color::PLAYER2.g, Color::PLAYER2.b);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 7);

    glUniform3f(1, Color::PLAYER1.r, Color::PLAYER1.g, Color::PLAYER1.b);
    glDrawArrays(GL_TRIANGLE_FAN, 7, 7);

    glDisableVertexAttribArray(0);
}

void Player::update()
{
    handleInput();
}

void Player::handleInput()
{
    if (Window::keyPressed(GLFW_KEY_D))
        move(0.015f);
    if (Window::keyPressed(GLFW_KEY_A))
        move(-0.015f);
}

void Player::move(GLfloat dx)
{
    translate.x += dx;
    translate.x = Math::clip(translate.x, -0.9f, 0.9f);
}

void Player::handleCollisions(Ball &ball)
{
    Triangle triangle = ball.getTriangle();

    for (int i = 0; i < 5; ++i)
        if (Triangle::intersect(triangle, ball.velocity, lines[i] + translate))
        {
            ball.bounce(lines[i] + translate, true);
            ball.shake();
            break;
        }
}
