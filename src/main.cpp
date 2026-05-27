#include "Game.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
#ifdef _WIN32
    // Enable UTF-8 output in Windows console.
    std::system("chcp 65001 > nul");
#endif

    std::cout << u8"\u6e38\u620f\u5df2\u542f\u52a8\uff1a\u65b9\u5411\u952e\u79fb\u52a8\uff0c\u9000\u51fa\u952e\u7ed3\u675f\u3002\n";

    bool autoStart = false;
    bool autoContinue = false;
    int demoExitLevel = 0;
    float demoExitHoldSeconds = 0.0F;
    std::string captureFrameDir;
    for (int index = 1; index < argc; ++index)
    {
        const std::string arg = argv[index];
        if (arg == "--autostart")
        {
            autoStart = true;
        }
        else if (arg == "--autocontinue")
        {
            autoContinue = true;
        }
        else if (arg == "--demo-exit-level" && index + 1 < argc)
        {
            demoExitLevel = std::max(0, std::atoi(argv[++index]));
        }
        else if (arg.rfind("--demo-exit-level=", 0) == 0)
        {
            demoExitLevel = std::max(0, std::atoi(arg.substr(18).c_str()));
        }
        else if (arg == "--demo-hold-seconds" && index + 1 < argc)
        {
            demoExitHoldSeconds = std::max(0.0F, (float)std::atof(argv[++index]));
        }
        else if (arg.rfind("--demo-hold-seconds=", 0) == 0)
        {
            demoExitHoldSeconds = std::max(0.0F, (float)std::atof(arg.substr(20).c_str()));
        }
        else if (arg == "--capture-dir" && index + 1 < argc)
        {
            captureFrameDir = argv[++index];
        }
        else if (arg.rfind("--capture-dir=", 0) == 0)
        {
            captureFrameDir = arg.substr(14);
        }
    }

    const int screenWidth = 1600;
    const int screenHeight = 960;
    InitWindow(screenWidth, screenHeight, u8"\u8857\u673a\u7834\u9635");
    if (demoExitLevel > 0)
    {
        SetWindowState(FLAG_WINDOW_TOPMOST);
    }
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight, autoStart, autoContinue, demoExitLevel, demoExitHoldSeconds, captureFrameDir);

    while (!game.ShouldClose())
    {
        game.HandleInput();
        game.Update();
        game.Draw();
    }

    CloseWindow();
    return 0;
}
