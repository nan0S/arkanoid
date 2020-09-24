#include "Board.hpp"

#include <iostream>

void Board::init(int row, int col, GLfloat side)
{
    this->row = row;
    this->col = col;

    top = {{1.5f, 1.0f}, {-1.5f, 1.0f}};
    bottom = {{-1.5f, -1.1f}, {1.5f, -1.1f}};
    left = {{-1.0f, 1.5f}, {-1.0f, -1.5f}};
    right = {{1.0f, -1.5f}, {1.0f, 1.5f}};
    
    triangles.resize(2 * row * col);

    GLfloat height = std::sqrt(3.0f) / 6.0f * side;
    glm::vec2 global_offset = glm::vec2(-((GLfloat)col + 0.5f) * side * 0.5f, 0.65f);
    
    for (int i = 0; i < row; ++i)
        for (int j = 0; j < col; ++j)   
        {
            int idx = 2 * (i * col + j);
            glm::vec2 local_offset = glm::vec2(j * side, -i * height * 3.0f - 3.0 * height * (i - 1));
            
            glm::vec2 local_offset1 = glm::vec2(side / 2.0f, height);    
            Triangle triangle(global_offset + local_offset + local_offset1, side * 0.8f, false);  
            triangles[idx] = triangle;

            local_offset1 = glm::vec2(side, 2.0f * height);
            triangle = Triangle(global_offset + local_offset + local_offset1, side * 0.8f, true);
            triangles[idx + 1] = triangle;
        }   

    glGenBuffers(1, &triangle_buffer);
}

void Board::draw()
{
    if (triangles.empty())
        return;

    glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
    glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(Triangle), &triangles[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform2f(0, 0.01f, -0.01f);
    glUniform3f(1, Color::BLACK.r, Color::BLACK.g, Color::BLACK.b);
    glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);
    
    glUniform2f(0, 0.0f, 0.0f);
    glUniform3f(1, Color::TRIANGLE.r, Color::TRIANGLE.g, Color::TRIANGLE.b);
    glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

bool Board::handleCollisions(Ball &ball)
{
    Triangle t = ball.getTriangle();

    if (Triangle::intersect(t, ball.velocity, bottom))
        return true;
    if (Triangle::intersect(t, ball.velocity, top))
        ball.bounce(top);
    if (Triangle::intersect(t, ball.velocity, left))
        ball.bounce(left);
    if (Triangle::intersect(t, ball.velocity, right))
        ball.bounce(right);

    Line l;
    for (auto it = triangles.begin(); it != triangles.end(); it++)
        if (Triangle::intersect(t, ball.velocity, *it, l))
        {
            triangles.erase(it);
            ball.bounce(l);
            break;
        }

    return false;
}

bool Board::empty()
{
    return triangles.empty();
}