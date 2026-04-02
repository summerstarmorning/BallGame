#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>
#include <iostream>
#include <cstdlib>

int main() {
    // 在 Windows 控制台中启用 UTF-8 输出，避免中文显示为乱码
#ifdef _WIN32
    // 使用 chcp 切换到 UTF-8（65001），避免包含 <windows.h> 导致与 raylib 冲突
    std::system("chcp 65001 > nul");
#endif

    // 简单的命令行说明（用于验证控制台中文输出）
    std::cout << "游戏启动：按左右方向键移动，按 Q 退出。\n";

    // 初始化窗口
    int screenWidth = 800;
    int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Break all the bricks with the ball");

    // 创建游戏对象
    Ball ball({400.0f, 300.0f}, {2.0f, 2.0f}, 10.0f);
    Paddle paddle(350.0f, 550.0f, 100.0f, 20.0f);

    // 创建砖块矩阵
    std::vector<Brick> bricks;
    float brickWidth = 100.0f;
    float brickHeight = 30.0f;
    for (int i = 0; i < 8; i++) {
        bricks.emplace_back(50.0f + i * 120.0f, 100.0f, brickWidth, brickHeight);
    }

    // 设置帧率
    SetTargetFPS(60);

    bool exitWindowRequest = false;

    // 游戏主循环
    while (!exitWindowRequest && !WindowShouldClose()) {
        // 按 Q 可以退出
        if (IsKeyPressed(KEY_Q)) {
            exitWindowRequest = true;
        }

        // 更新物理与位置
        ball.Move();
        ball.BounceEdge(screenWidth, screenHeight);

        // 球与挡板碰撞检测
        if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {
            Vector2 speed = ball.GetSpeed();
            if (speed.y > 0) {
                // 只在向下运动时反弹
                speed.y = -speed.y;
                float paddleCenter = paddle.GetRect().x + paddle.GetRect().width / 2.0f;
                float hitOffset = ball.GetPosition().x - paddleCenter;
                // 根据击中偏移改变 x 分量
                speed.x += hitOffset * 0.05f;
                ball.SetSpeed(speed);
            }
        }

        // 球与砖块碰撞检测
        for (auto& brick : bricks) {
            if (brick.IsActive() && CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), brick.GetRect())) {
                brick.SetActive(false);

                Vector2 speed = ball.GetSpeed();
                Vector2 pos = ball.GetPosition();
                Rectangle rect = brick.GetRect();

                bool hitSide = (pos.x < rect.x) || (pos.x > rect.x + rect.width);
                bool hitTopBottom = (pos.y < rect.y) || (pos.y > rect.y + rect.height);

                if (hitSide && !hitTopBottom) {
                    speed.x = -speed.x;
                } else if (!hitSide && hitTopBottom) {
                    speed.y = -speed.y;
                } else {
                    speed.x = -speed.x;
                    speed.y = -speed.y;
                }

                ball.SetSpeed(speed);
                break; // 每帧只处理一次砖块碰撞
            }
        }

        // 挡板控制
        if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5.0f);
        if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5.0f);

        // 绘制
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 边界
        DrawRectangle(0, 0, 5, screenHeight, GRAY);
        DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
        DrawRectangle(0, 0, screenWidth, 5, GRAY);
        DrawRectangle(0, screenHeight - 5, screenWidth, 5, GRAY);

        // 绘制游戏对象
        ball.Draw();
        paddle.Draw();
        for (auto& brick : bricks) {
            brick.Draw();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
