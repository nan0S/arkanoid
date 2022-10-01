#include "Game.hpp"

void Game::start()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return;
    }

    if (!Window::create(1080, 1080, "Arkanoid"))
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return;
    }

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return;
    }

    initGame();
    run();
}

void Game::initGame()
{
    standard_shader = Shader::load("../shader/standardVertexShader.glsl", "../shader/standardFragmentShader.glsl");
    if (!standard_shader)
    {
       fprintf(stderr, "Failed to load standard shader.\n");
       exit(EXIT_FAILURE);
    }

    bg_shader = Shader::load("../shader/bgVertexShader.glsl", "../shader/bgFragmentShader.glsl");
    if (!bg_shader)
    {
       fprintf(stderr, "Failed to load background shader.\n");
       exit(EXIT_FAILURE);
    }

   glm::vec2 points[] = {
      { -1.0f, 1.0f },
      { -1.0f, -1.0f },
      { 1.0f, 1.0f },
      { 1.0f, -1.0f }
   };

    glGenBuffers(1, &bg_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, bg_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), points, GL_STATIC_DRAW);

    level = 5;
    restart();
}

void Game::restart(bool finished)
{
    last_space_state = 0;
    paused = false;
    pause_time = 0.0;

    if (finished)
        ++level;

    player.init();
    ball.init();
    board.init(level, 5 * level / 2, 0.11f);
}

void Game::run()
{
    GLuint vertexArrayID;
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glUseProgram(standard_shader);

    while (!Window::windowShouldClose())
    {
        handleInput();
        if (!paused)
        {
            checkCollisions();
            player.update();
            ball.update();
            draw();
        }
        if (levelFinished())
            restart(true);

        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteProgram(standard_shader);
    glDeleteProgram(bg_shader);
    glfwTerminate();
}

void Game::draw()
{
    Window::clear();

    glUseProgram(bg_shader);

    glBindBuffer(GL_ARRAY_BUFFER, bg_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) , (void*)0);
    glEnableVertexAttribArray(0);
    glUniform1f(glGetUniformLocation(bg_shader, "time"), glfwGetTime());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(standard_shader);
    board.draw();
    ball.draw();
    player.draw();

    Window::display();
}

void Game::handleInput()
{
    if (Window::keyPressed(GLFW_KEY_N))
        restart();

    int space_state = glfwGetKey(Window::window, GLFW_KEY_SPACE);
    if (space_state && !last_space_state)
    {
        if (paused)
            std::cout << "Game resumed!\n",
            glfwSetTime(pause_time);
        else
            std::cout << "Game paused\n",
            pause_time = glfwGetTime();
        paused = !paused;
    }
    last_space_state = space_state;

    // static double last_time = glfwGetTime();
    // static int last_g_state = 0;
    // int g_state = glfwGetKey(Window::window, GLFW_KEY_G);
    // if (g_state)
    // {
    //     if (!last_g_state)
    //         glfwSetTime(last_time);
    //     loop();
    // }
    // else
    // {
    //     if (last_g_state)
    //         last_time = glfwGetTime();
    // }
    // last_g_state = g_state;
}

void Game::checkCollisions()
{
    if (board.handleCollisions(ball))
    {
        restart();
        return;
    }

    player.handleCollisions(ball);
}

bool Game::levelFinished()
{
    return board.empty();
}
