#pragma once

#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

class Game {
private:
    //屏幕大小
    //游戏对象：小球、挡板、砖块
    int screenWidth;
    int screenHeight;
    Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;
    //游戏状态
    //生命值、分数、是否运行中、是否胜利、退出请求
    int lives;
    int score;
    enum class GameState { MENU, PLAYING, PAUSED, GAMEOVER };
    GameState currentState;
    bool victory;
    bool exitWindowRequest;
    //碰撞参数
    //碰撞偏移量、挡板影响力、最大水平速度、上一帧挡板位置
    const float HIT_OFFSET_FACTOR = 4.0f;    // 减弱边缘击球带来的增速幅度
    const float PADDLE_INFLUENCE = 0.2f;     // 减弱挡板自身速度带来的附加初速度
    const float MAX_H_SPEED = 10.0f;         // 稍微限制最大横向速度，避免过快
    float prevPaddleX;
    //私有方法：初始化砖块、检查底部碰撞、处理挡板碰撞、处理砖块碰撞
    void InitBricks();
    void CheckBottomCollision();
    void HandlePaddleCollision(float paddleVel);
    void HandleBrickCollision();

public:
    //构造函数：屏幕大小
    Game(int width, int height);
    //接口：是否请求关闭窗口、处理输入、更新游戏状态、渲染
    bool ShouldClose() const;
    void HandleInput();
    void Update();
    void Draw();
};
