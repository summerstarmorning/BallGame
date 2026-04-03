#include "Game.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include "nlohmann_json.hpp"

using json = nlohmann::json;

// 构造函数：初始化屏幕大小
// 球位置和速度
// 挡板位置
// 生命值、分数、游戏状态等
Game::Game(int width, int height)
    : screenWidth(width), screenHeight(height),
      ball({(float)width / 2.0f, (float)height / 2.0f}, {4.0f, 4.0f}, 15.0f),
      paddle((float)width / 2.0f - 75.0f, (float)height - 68.0f, 150.0f, 25.0f),
      lives(3), score(0), currentState(GameState::MENU), victory(false), exitWindowRequest(false),
      isDarkMode(false), ballColorIndex(0), paddleColorIndex(0), brickColorIndex(0)
{
    // 定义可选的颜色
    Color colors[] = {RED, BLUE, GREEN, ORANGE, PURPLE, MAROON, DARKBLUE, DARKGREEN};
    ballColor = colors[ballColorIndex];
    paddleColor = colors[paddleColorIndex + 1];
    brickColor = colors[brickColorIndex + 2];

    std::ifstream f("config.json");
    if (f.is_open()) {
        try {
            // 参数四为 true 允许解析注释
            json data = json::parse(f, nullptr, true, true);
            
            if (data.contains("game")) {
                lives = data["game"].value("lives", 3);
            }
            if (data.contains("ball")) {
                ball = Ball(
                    {data["ball"].value("startX", 512.0f), data["ball"].value("startY", 384.0f)},
                    {data["ball"].value("speedX", 4.0f), data["ball"].value("speedY", 4.0f)},
                    data["ball"].value("radius", 15.0f)
                );
            }
            if (data.contains("paddle")) {
                paddle = Paddle(
                    data["paddle"].value("startX", 437.0f),
                    data["paddle"].value("startY", 700.0f),
                    data["paddle"].value("width", 150.0f),
                    data["paddle"].value("height", 25.0f)
                );
            }
        } catch (json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        }
    }

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
    Vector2 mousePos = GetMousePosition();
    if (IsKeyPressed(KEY_Q)) {
        exitWindowRequest = true;
    }

    if (currentState == GameState::MENU) {
        // 主界面颜色修改按钮尺寸
        Rectangle themeBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 60, 200, 40 };
        Rectangle ballBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 110, 200, 40 };
        Rectangle paddleBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 160, 200, 40 };
        Rectangle brickBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 210, 200, 40 };
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, themeBtn)) isDarkMode = !isDarkMode;
            if (CheckCollisionPointRec(mousePos, ballBtn)) ballColorIndex = (ballColorIndex + 1) % 8;
            if (CheckCollisionPointRec(mousePos, paddleBtn)) paddleColorIndex = (paddleColorIndex + 1) % 8;
            if (CheckCollisionPointRec(mousePos, brickBtn)) brickColorIndex = (brickColorIndex + 1) % 8;
        }

        // Color Palette
        Color colors[] = {RED, BLUE, GREEN, ORANGE, PURPLE, MAROON, DARKBLUE, DARKGREEN};
        ballColor = colors[ballColorIndex];
        paddleColor = colors[paddleColorIndex];
        brickColor = colors[brickColorIndex];

        if (IsKeyPressed(KEY_ENTER)) currentState = GameState::PLAYING;
    } else if (currentState == GameState::PLAYING) {
        Rectangle pauseBtn = { screenWidth - 60.0f, 10.0f, 40.0f, 40.0f }; // 右上角暂停按钮区域

        if (IsKeyPressed(KEY_P)) currentState = GameState::PAUSED;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, pauseBtn)) {
            currentState = GameState::PAUSED;
        }

        if (!victory) {
            if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5.0f);
            if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5.0f);
        }
    } else if (currentState == GameState::PAUSED) {
        Vector2 mousePos = GetMousePosition();
        Rectangle resumeBtn = { screenWidth - 60.0f, 10.0f, 40.0f, 40.0f }; // 右上角暂停按钮区域

        if (IsKeyPressed(KEY_P)) currentState = GameState::PLAYING;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, resumeBtn)) {
            currentState = GameState::PLAYING;
        }
    } else if (currentState == GameState::GAMEOVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            // Restart game logic can be complex, let's just allow quit for now
            // exitWindowRequest = true;
        }
    }
}
// 更新游戏状态：位置更新、碰撞检测、胜利条件等
void Game::Update() {
    if (currentState != GameState::PLAYING || victory) return;

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
            currentState = GameState::GAMEOVER;
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
            // 移除了依赖不同击球位置产生不同速度的设定
            // PADDLE_INFLUENCE 控制挡板移动对速度的影响（切向速度保留）
            speed.x += paddleVel * PADDLE_INFLUENCE;
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
    
    // 渲染背景
    ClearBackground(isDarkMode ? BLACK : RAYWHITE);
    Color textColor = isDarkMode ? LIGHTGRAY : DARKGRAY;
    Color borderColor = isDarkMode ? DARKGRAY : GRAY;

    // 边界
    DrawRectangle(0, 0, 5, screenHeight, borderColor);
    DrawRectangle(screenWidth - 5, 0, 5, screenHeight, borderColor);
    DrawRectangle(0, 0, screenWidth, 5, borderColor);

    // UI部分
    DrawText(TextFormat("SCORE: %d", score), 20, 20, 20, textColor);
    DrawText(TextFormat("LIVES: %d", lives), screenWidth - 180, 20, 20, MAROON); // 移至左侧避开暂停按钮

    // 暂停/继续按钮图形绘制
    Rectangle btnRec = { screenWidth - 60.0f, 10.0f, 40.0f, 40.0f };
    DrawRectangleRec(btnRec, isDarkMode ? DARKGRAY : LIGHTGRAY);
    DrawRectangleLinesEx(btnRec, 2, textColor);
    if (currentState == GameState::PLAYING) {
        // 画暂停图标 (||)
        DrawRectangle(btnRec.x + 12, btnRec.y + 10, 6, 20, textColor);
        DrawRectangle(btnRec.x + 22, btnRec.y + 10, 6, 20, textColor);
    } else {
        // 画播放图标 (三角形)
        Vector2 p1 = { btnRec.x + 14, btnRec.y + 10 };
        Vector2 p2 = { btnRec.x + 14, btnRec.y + 30 };
        Vector2 p3 = { btnRec.x + 32, btnRec.y + 20 };
        DrawTriangle(p1, p2, p3, textColor);
    }

    if (currentState == GameState::MENU) {
        DrawText("BRICK BREAKER", screenWidth / 2 - 130, screenHeight / 2 - 120, 40, isDarkMode ? RAYWHITE : DARKBLUE);
        DrawText("Press ENTER to Start", screenWidth / 2 - 100, screenHeight / 2 - 60, 20, textColor);
        
        // 主角颜色配置UI
        Rectangle themeBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 60, 200, 40 };
        Rectangle ballBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 110, 200, 40 };
        Rectangle paddleBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 160, 200, 40 };
        Rectangle brickBtn = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 210, 200, 40 };
        
        DrawRectangleRounded(themeBtn, 0.2f, 4, isDarkMode ? LIGHTGRAY : GRAY);
        DrawText(isDarkMode ? "Theme: DARK" : "Theme: LIGHT", (int)(themeBtn.x + 40), (int)(themeBtn.y + 10), 20, isDarkMode ? BLACK : WHITE);

        DrawRectangleRounded(ballBtn, 0.2f, 4, ballColor);
        DrawText("Ball Color", (int)(ballBtn.x + 50), (int)(ballBtn.y + 10), 20, WHITE);

        DrawRectangleRounded(paddleBtn, 0.2f, 4, paddleColor);
        DrawText("Paddle Color", (int)(paddleBtn.x + 35), (int)(paddleBtn.y + 10), 20, WHITE);

        DrawRectangleRounded(brickBtn, 0.2f, 4, brickColor);
        DrawText("Brick Color", (int)(brickBtn.x + 45), (int)(brickBtn.y + 10), 20, WHITE);

    } else if (currentState == GameState::PAUSED) {
        DrawText("PAUSED", screenWidth / 2 - 70, screenHeight / 2, 40, ORANGE);
        DrawText("Press P to Resume", screenWidth / 2 - 100, screenHeight / 2 + 50, 20, textColor);
    } else if (currentState == GameState::GAMEOVER) {
        DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2, 40, RED);
        DrawText("Press Q to Quit", screenWidth / 2 - 120, screenHeight / 2 + 50, 20, textColor);
    } else if (victory) {
        DrawText("YOU WIN!", screenWidth / 2 - 100, screenHeight / 2, 40, GREEN);
        DrawText(TextFormat("FINAL SCORE: %d", score), screenWidth / 2 - 120, screenHeight / 2 + 50, 20, textColor);
        DrawText("Press Q to Quit", screenWidth / 2 - 120, screenHeight / 2 + 80, 20, textColor);
    } else {
        // 实体
        ball.Draw(ballColor);
        paddle.Draw(paddleColor);
        for (auto& brick : bricks) {
            brick.Draw(brickColor);
        }
    }

    EndDrawing();
}
