#include "Game.h"
#include <cmath>
// 构造函数：初始化屏幕大小
// 球位置和速度
// 挡板位置
// 生命值、分数、游戏状态等
Game::Game(int width, int height)
    : screenWidth(width), screenHeight(height),
      ball({512.0f, 384.0f}, {4.0f, 4.0f}, 15.0f),       // 增大球（半径由10调整为15）及初始速度调整
      paddle(437.0f, 700.0f, 150.0f, 25.0f),             // 挡板变宽(100->150)变厚，位置调整以适应大屏
      lives(3), score(0), gameRunning(true), victory(false), exitWindowRequest(false)
{
    InitBricks();
    prevPaddleX = paddle.GetRect().x;
}
// 私有方法：初始化砖块
void Game::InitBricks() {
    float brickWidth = 120.0f;
    float brickHeight = 40.0f;
    for (int i = 0; i < 8; i++) {
        // 放大砖块体积(120*40)，横向占更宽
        bricks.emplace_back(30.0f + i * 122.0f, 120.0f, brickWidth, brickHeight);
    }
}
// 是否请求关闭窗口
bool Game::ShouldClose() const {
    return exitWindowRequest || WindowShouldClose();
}
// 处理输入
void Game::HandleInput() {
    if (IsKeyPressed(KEY_Q)) {
        exitWindowRequest = true;
    }

    if (gameRunning && !victory) {
        if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5.0f);
        if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5.0f);
    }
}
// 更新游戏状态：位置更新、碰撞检测、胜利条件等
void Game::Update() {
    if (!gameRunning || victory) return;

    float paddleVel = paddle.GetRect().x - prevPaddleX;
    
    // 外部边界物理与位置更新
    ball.Move();
    ball.BounceEdge(screenWidth, screenHeight);

    CheckBottomCollision();
    HandlePaddleCollision(paddleVel);
    HandleBrickCollision();
    
    // 更新上一帧挡板位置
    prevPaddleX = paddle.GetRect().x;
}
// 渲染：清屏、绘制实体、UI、结束绘制
void Game::CheckBottomCollision() {
    if (ball.GetPosition().y + ball.GetRadius() >= screenHeight) {
        lives--;
        if (lives <= 0) {
            gameRunning = false;
        } else {
            ball.SetPosition({ screenWidth / 2.0f, screenHeight / 2.0f }); // 发球点居中
            ball.SetSpeed({ 4.0f, 4.0f });
        }
    }
}
// 处理挡板碰撞：检测碰撞，调整速度，考虑偏移和挡板移动影响
void Game::HandlePaddleCollision(float paddleVel) {
    if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {
        Vector2 speed = ball.GetSpeed();
        if (speed.y > 0) {
            speed.y = -std::abs(speed.y); // 防止卡弹反转
            // 根据球相对于挡板中心的偏移调整水平速度
            float paddleCenter = paddle.GetRect().x + paddle.GetRect().width / 2.0f;
            float normalized = (ball.GetPosition().x - paddleCenter) / (paddle.GetRect().width / 2.0f);
            // HIT_OFFSET_FACTOR 控制偏移对速度的影响程度
            // PADDLE_INFLUENCE 控制挡板移动对速度的影响
            speed.x += normalized * HIT_OFFSET_FACTOR + paddleVel * PADDLE_INFLUENCE;
            // 限制最大水平速度，防止过快导致难以控制
            if (speed.x > MAX_H_SPEED) speed.x = MAX_H_SPEED;
            if (speed.x < -MAX_H_SPEED) speed.x = -MAX_H_SPEED;
            // 更新球的速度
            ball.SetSpeed(speed);
        }
    }
}

void Game::HandleBrickCollision() {
    int activeBricksCount = 0;
    for (auto& brick : bricks) {
        if (!brick.IsActive()) continue;
        
        activeBricksCount++;
        if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), brick.GetRect())) {
            brick.SetActive(false);
            score += 100;
            activeBricksCount--;

            Vector2 speed = ball.GetSpeed();
            Vector2 pos = ball.GetPosition();
            Rectangle rect = brick.GetRect();

            bool hitSide = (pos.x < rect.x) || (pos.x > rect.x + rect.width);
            bool hitTopBottom = (pos.y < rect.y) || (pos.y > rect.y + rect.height);

            if (hitSide && !hitTopBottom) speed.x = -speed.x;
            else if (!hitSide && hitTopBottom) speed.y = -speed.y;
            else { speed.x = -speed.x; speed.y = -speed.y; }

            ball.SetSpeed(speed);
            break; // 每次只处理一次砖块碰撞
        }
    }

    if (activeBricksCount == 0) {
        victory = true;
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // 边界
    DrawRectangle(0, 0, 5, screenHeight, GRAY);
    DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
    DrawRectangle(0, 0, screenWidth, 5, GRAY);

    // UI部分
    DrawText(TextFormat("SCORE: %d", score), 20, 20, 20, DARKGRAY);
    DrawText(TextFormat("LIVES: %d", lives), screenWidth - 120, 20, 20, MAROON);

    if (!gameRunning) {
        DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2, 40, RED);
        DrawText("Press Q to Quit", screenWidth / 2 - 120, screenHeight / 2 + 50, 20, GRAY);
    } else if (victory) {
        DrawText("YOU WIN!", screenWidth / 2 - 100, screenHeight / 2, 40, GREEN);
        DrawText(TextFormat("FINAL SCORE: %d", score), screenWidth / 2 - 120, screenHeight / 2 + 50, 20, GRAY);
        DrawText("Press Q to Quit", screenWidth / 2 - 120, screenHeight / 2 + 80, 20, GRAY);
    } else {
        // 实体
        ball.Draw();
        paddle.Draw();
        for (auto& brick : bricks) {
            brick.Draw();
        }
    }

    EndDrawing();
}
