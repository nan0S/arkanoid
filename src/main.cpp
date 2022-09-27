#include "Game.hpp"

int main()
{
    Game game;
    game.start();

    return 0;
}

int main2()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return EXIT_FAILURE;
    }

    if (!Window::create(1080, 1080, "Arkanoid"))
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        return EXIT_FAILURE;
    }

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        return EXIT_FAILURE;
    }

    GLuint game_shader = Load
    initGame();
    run();

    return EXIT_SUCCESS;
}
