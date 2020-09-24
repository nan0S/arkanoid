#include "Line.hpp"

bool Line::intersect(Line &l1, Line &l2)  
{
    Point &m_p1 = l1.points[0], &m_p2 = l1.points[1];
    Point &o_p1 = l2.points[0], &o_p2 = l2.points[1];
 
    int o1 = Line::orientation(m_p1, m_p2, o_p1);
    int o2 = Line::orientation(m_p1, m_p2, o_p2);
    int o3 = Line::orientation(o_p1, o_p2, m_p1);
    int o4 = Line::orientation(o_p1, o_p2, m_p2);
    
    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && Line::onSegment(m_p1, o_p1, m_p2))
        return true;
    if (o2 == 0 && Line::onSegment(m_p1, o_p2, m_p2))
        return true;
    if (o3 == 0 && Line::onSegment(o_p1, m_p1, o_p2))
        return true;
    if (o4 == 0 && Line::onSegment(o_p1, m_p2, o_p2))
        return true;
    
    return false;
}

int Line::orientation(Point p1, Point p2, Point p3)
{
    GLfloat value = Point::cross(p1 - p2, p3 - p2);
    if (value == 0)
        return 0;
    return value > 0 ? 1 : 2;
}

bool Line::onSegment(Point p1, Point p2, Point p3)
{
    return p2.x >= std::min(p1.x, p3.x) && p2.x <= std::max(p1.x, p3.x) && 
            p2.y >= std::min(p1.y, p3.y) && p2.y <= std::max(p1.y, p3.y);
}

void Line::init(Color color)
{
    time = 0.0f;
    vertex_count = 2;

    glGenBuffers(1, &points_buffer);
    onPositionChange();

    colors = new Color[2]{color, color};
    glGenBuffers(1, &colors_buffer);
    onColorChange();
}



