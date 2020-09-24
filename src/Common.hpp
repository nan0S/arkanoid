#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <random>
#include <iostream>

struct Color
{
    GLfloat r, g, b;

    static Color RED;
    static Color BLUE;
    static Color GREEN;
    static Color YELLOW;
    static Color PURPLE;
    static Color ORANGE;
    static Color BLACK;
    static Color NONE;
    static Color RAINBOW;
    static Color DARK_PURPLE;
    static Color DARK_YELLOW;
    static Color WHITE;
    static Color BG;
    static Color PLAYER1;
    static Color PLAYER2;
    static Color BALL;
    static Color TRIANGLE;

    bool operator== (Color color)
    {
        return this->r == color.r && this->g == color.g && this->b == color.b;
    }
};

class Random
{
private:
    static std::mt19937 gen;

public:
    template<typename T>
    static T random(T min, T max)
    {
        using dist = std::conditional_t<
            std::is_integral<T>::value,
            std::uniform_int_distribution<T>,
            std::uniform_real_distribution<T>
        >;
        return dist{min, max}(gen);
    }

    static GLfloat randnorm()
    {
        return random(-1.0f, 1.0f);
    }
};

class Math
{
public:

    template<typename T>
    static T constrain(T value, T value_min, T value_max, T target_min, T target_max)
    {
        T value_length = value_max - value_min;
        T ratio = (value - value_min) / value_length;
        return target_min + (target_max - target_min) * ratio;
    }

    template<typename T>
    static T clip(T value, T min, T max)
    {
        return std::min(std::max(min, value), max);
    }

    static GLfloat vec2Length(const glm::vec2 &v)
    {
        return sqrt(v.x * v.x + v.y * v.y);
    }
    
    static glm::vec2 normalized(glm::vec2 v)
    {
        v /= vec2Length(v);
        return v;
    }

    static GLfloat cross(glm::vec2 v1, glm::vec2 v2)
    {
        return v1.x * v2.x + v1.y * v2.y;
    }

    static void cout(glm::vec2 v)
    {
        std::cout << "(" << v.x << ", " << v.y << ")";
    }
};
