#include "Common.hpp"
#include <random>

Color Color::RED = {1.0f, 0.0f, 0.0f};
Color Color::BLUE = {0.0f, 0.0f, 1.0f};
Color Color::GREEN = {0.0f, 1.0f, 0.0f};
Color Color::YELLOW = {1.0f, 1.0f, 0.2f};
Color Color::PURPLE = {0.9f, 0.0f, 0.8f};
Color Color::ORANGE = {1.0f, 0.5f, 0.0f};
Color Color::BLACK = {0.0f, 0.0f, 0.0f};
Color Color::NONE = {-1.0f, -1.0f, -1.0f};
Color Color::RAINBOW = {83.1f / 255, 68.6f / 255, 21.6f / 255};
Color Color::DARK_PURPLE = {128.0f / 255, 0.0f, 128.0f / 255};
Color Color::DARK_YELLOW = {150.0f / 255, 150.0f / 255, 0.0f};
Color Color::WHITE = {1.0f, 1.0f, 1.0f};
// Color Color::BG = { 7.0f / 255, 30.0f / 255, 34.0f / 255};
// Color Color::PLAYER1 = { 29.0f / 255, 120.0f / 255, 116.0f / 255};
// Color Color::PLAYER2 = { 103.0f / 255, 146.0f / 255, 137.0f / 255};
// Color Color::BALL = { 244.0f / 255, 192.0f / 255, 149.0f / 255};
// Color Color::TRIANGLE = { 238.0f / 255, 46.0f / 255, 49.0f / 255};
Color Color::BG = { 7.0f / 255, 30.0f / 255, 34.0f / 255};
Color Color::PLAYER1 = { 24.0f / 255, 100.0f / 255, 97.0f / 255};
Color Color::PLAYER2 = { 80.0f / 255, 120.0f / 255, 111.0f / 255};
Color Color::BALL = { 244.0f / 255, 192.0f / 255, 149.0f / 255};
Color Color::TRIANGLE = { 150.0f / 255, 34.0f / 255, 37.0f / 255};

std::mt19937 Random::gen{std::random_device{}()};