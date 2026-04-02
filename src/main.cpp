#include "Game.h"
#include <iostream>
#include <cstdlib>

int main() {
    // 在 Windows 控制台中启用 UTF-8 输出，避免中文显示为乱码
#ifdef _WIN32
    std::system("chcp 65001 > nul");
#endif

    std::cout << "游戏启动：按左右方向键移动，按 Q 退出。\n";

    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "Break all the bricks with the ball");
    SetTargetFPS(60);

    // 实例化游戏类
    Game game(screenWidth, screenHeight);

    // 核心必要的主游戏循环
    while (!game.ShouldClose()) {
        game.HandleInput();  // 处理玩家输入
        game.Update();       // 游戏逻辑更新（位置、物理、碰撞）
        game.Draw();         // 画面渲染
    }

    CloseWindow();
    return 0;
}
