#include "Game.h"

#include <cstdlib>
#include <iostream>

int main()
{
#ifdef _WIN32
    // Enable UTF-8 output in Windows console.
    std::system("chcp 65001 > nul");
#endif

    std::cout << "Game started: move with arrow keys, press Q to quit.\n";

    const int screenWidth = 1600;
    const int screenHeight = 960;
    InitWindow(screenWidth, screenHeight, "Break all the bricks with the ball");
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);

    while (!game.ShouldClose())
    {
        game.HandleInput();
        game.Update();
        game.Draw();
    }

    CloseWindow();
    return 0;
}
