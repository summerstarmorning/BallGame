#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

int main() {
    // 初始化窗口
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Break all the bricks with the ball");
    // 球（初始位置，初始速度，半径）
    // 挡板（初始位置，宽高）
    Ball ball({400.0f, 300.0f}, {2.0f, 2.0f}, 10.0f);
    Paddle paddle(350.0f, 550.0f, 100.0f, 20.0f);
    // 砖块（初始位置，宽高）
    std::vector<Brick> bricks;
    float brickWidth = 100.0f;
    float brickHeight = 30.0f;
    
    for (int i = 0; i < 8; i++) {
        bricks.emplace_back(50.0f + i * 120.0f, 100.0f, brickWidth, brickHeight);
    }
    //设置帧率
    SetTargetFPS(60);

    // 主循环控制标志，用于演示优雅退出
    bool exitWindowRequest = false;

    //主循环
    // WindowShouldClose(): 检查是否收到关闭窗口请求（如点击关闭按钮或按下 ESC 键）
    while (!exitWindowRequest && !WindowShouldClose()) {
        // 如果玩家按下 Q 键请求手动退出：也可在此作提示或存档
        if (IsKeyPressed(KEY_Q)) {
            exitWindowRequest = true; // 优雅退出并结束循环
        }

    //无驱动运行:
        //小球的运动
        //持续不断地碰撞检测
        //监测玩家输入
        //渲染
        ball.Move();
        ball.BounceEdge(screenWidth, screenHeight);
        //左右输入控制paddle移动
        if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5.0f);
        if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5.0f);
    //开始渲染
        // BeginDrawing(): 设置渲染上下文，准备并开始绘制这一帧的内容
        BeginDrawing();
        // ClearBackground(): 擦除上一帧画面，用预定义的背景色RAYWHITE（浅白）填充
        ClearBackground(RAYWHITE);
        //渲染walls 5个像素灰
        DrawRectangle(0, 0, 5, screenHeight, GRAY);
        DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
        DrawRectangle(0, 0, screenWidth, 5, GRAY);
        DrawRectangle(0, screenHeight - 5, screenWidth, 5, GRAY);
        //渲染ball paddle bricks
        ball.Draw();
        paddle.Draw();
        for (auto& brick : bricks) {
            brick.Draw();
        }
    //结束渲染
        // EndDrawing(): 结束并提交当前帧的渲染，将其交换到屏幕显示
        EndDrawing();
    }
    //循环条件只是检测，实际关闭还需要调用CloseWindow()
    // CloseWindow(): 关闭整个应用窗口，卸载关联的系统与显卡资源
    CloseWindow();
    return 0;
}