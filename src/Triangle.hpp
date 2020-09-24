#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "Common.hpp"

struct Line
{  
    glm::vec2 p1;
    glm::vec2 p2;

    Line() {}
    Line(glm::vec2 p1, glm::vec2 p2) : p1(p1), p2(p2) {}
  
    static bool intersect(const Line &l1, const Line &l2);
    static int orientation(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3);
    static bool onSegment(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3);
    static GLfloat cross(const glm::vec2 p1, const glm::vec2 p2);

    Line operator+(const glm::vec2 &v) const { return {p1 + v, p2 + v}; }

    friend std::ostream& operator<<(std::ostream &os, const Line &l)
    {
        os << "(" << l.p1.x << ", " << l.p1.y << ") ---- (" << l.p2.x << ", " << l.p2.y << ")\n";
        return os; 
    }
};


struct Triangle
{
    glm::vec2 p1;
    glm::vec2 p2;
    glm::vec2 p3;

    Triangle() {}
    Triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) : p1(p1), p2(p2), p3(p3) {}
    Triangle(glm::vec2 p, GLfloat side_length, bool inverted);

    static bool intersect(const Triangle &t1, const glm::vec2 &velocity, const Triangle &t2, Line &l);
    static bool intersect(const Triangle &t, const glm::vec2 &velocity, const Line l);
    static bool isInside(Triangle t1, Triangle t2);

    Triangle operator+(const glm::vec2 &v) const { return {p1 + v, p2 + v, p3 + v}; }

    friend std::ostream& operator<<(std::ostream &os, const Triangle &t)
    {
        os << "(" << t.p1.x << ", " << t.p1.y << ") "
           << "(" << t.p2.x << ", " << t.p2.y << ") " 
           << "(" << t.p3.x << ", " << t.p3.y << ") ";
        return os;
    }
};