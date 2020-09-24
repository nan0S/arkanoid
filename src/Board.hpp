#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Triangle.hpp"
#include "Ball.hpp"

class Board
{
private:
    GLuint triangle_buffer;
    std::vector<Triangle> triangles;

    int row;
    int col;

    Line top, bottom;
    Line left, right;

public:
    Board() {}
    
    void init(int row, int col, GLfloat side);
    void draw();
    bool handleCollisions(Ball &ball);
    bool empty();
};