#include "Triangle.hpp"

bool Line::intersect(const Line &l1, const Line &l2)  
{ 
    int o1 = Line::orientation(l1.p1, l1.p2, l2.p1);
    int o2 = Line::orientation(l1.p1, l1.p2, l2.p2);
    int o3 = Line::orientation(l2.p1, l2.p2, l1.p1);
    int o4 = Line::orientation(l2.p1, l2.p2, l1.p2);
    
    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && Line::onSegment(l1.p1, l2.p1, l1.p2))
        return true;
    if (o2 == 0 && Line::onSegment(l1.p1, l2.p2, l1.p2))
        return true;
    if (o3 == 0 && Line::onSegment(l2.p1, l1.p1, l2.p2))
        return true;
    if (o4 == 0 && Line::onSegment(l2.p1, l1.p2, l2.p2))
        return true;
    
    return false;
}

int Line::orientation(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3)
{
    GLfloat value = Line::cross(p1 - p2, p3 - p2);
    if (value == 0)
        return 0;
    return value > 0 ? 1 : 2;
}

bool Line::onSegment(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3)
{
    return p2.x >= std::min(p1.x, p3.x) && p2.x <= std::max(p1.x, p3.x) && 
            p2.y >= std::min(p1.y, p3.y) && p2.y <= std::max(p1.y, p3.y);
}

GLfloat Line::cross(const glm::vec2 p1, const glm::vec2 p2)
{
    return p1.x * p2.y - p1.y * p2.x;
}


Triangle::Triangle(glm::vec2 p, GLfloat side_length, bool inverted = true)
{
    GLfloat height = side_length * std::sqrt(3.0f) / 6.0f;
    if (inverted)
    {
        p1 = glm::vec2(p.x - 0.5f * side_length, p.y + height);
        p2 = glm::vec2(p.x + 0.5f * side_length, p.y + height);
        p3 = glm::vec2(p.x, p.y - 2.0f * height);
    }
    else
    {
        p1 = glm::vec2(p.x - 0.5f * side_length, p.y - height);
        p2 = glm::vec2(p.x, p.y + 2.0f * height); 
        p3 = glm::vec2(p.x + 0.5f * side_length, p.y - height);
    }
}

bool Triangle::intersect(const Triangle &t1, const glm::vec2 &velocity, const Triangle &t2, Line &l)
{
    Line lines[3] = {Line(t2.p1, t2.p2), Line(t2.p2, t2.p3), Line(t2.p3, t2.p1)};

    for (int i = 0; i < 3; ++i)
        if (Triangle::intersect(t1, velocity, lines[i]))
        {
            l = lines[i];
            return true;
        }

    return (Triangle::isInside(t1, t2) || Triangle::isInside(t2, t1));
}

bool Triangle::intersect(const Triangle &t, const glm::vec2 &velocity, const Line l)
{
    glm::vec2 normal = l.p2 - l.p1;
    normal = glm::vec2(-normal.y, normal.x);

    if (Math::cross(velocity, normal) > 0.0f)  
        return false;

    Line l1 = Line(t.p1, t.p2);
    Line l2 = Line(t.p2, t.p3);
    Line l3 = Line(t.p3, t.p1);

    return Line::intersect(l1, l) || Line::intersect(l2, l) || Line::intersect(l3, l);
}

bool Triangle::isInside(Triangle t1, Triangle t2)
{
    glm::vec2 &p = t1.p1;
    glm::vec2 &p1 = t2.p1, &p2 = t2.p2, &p3 = t2.p3;

    bool s1 = std::signbit(Line::cross(p - p1, p2 - p1));
    bool s2 = std::signbit(Line::cross(p - p2, p3 - p2));
    bool s3 = std::signbit(Line::cross(p - p3, p1 - p3));

    return ((s1 && s2 && s3) || (!s1 && !s2 && !s3));    
}