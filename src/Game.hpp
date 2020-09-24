#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "Window.hpp"
#include "Player.hpp"
#include "Shader.hpp"
#include "Ball.hpp"
#include "Board.hpp"

class Game
{
private:
    GLuint standard_shader;
    GLuint bg_shader;

    GLuint bg_buffer;

    Player player;
    Ball ball;
    Board board;

    int last_space_state;
    bool paused;
    double pause_time;

    int level;

    void initGame();
    void run();
    void restart(bool finished = false);
    void draw();
    void handleInput();
    void checkCollisions();
    bool levelFinished();

public: 
    Game() {}

    void start();
};