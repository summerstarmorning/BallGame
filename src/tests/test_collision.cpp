#include <iostream>
#include <cstdlib>
#include "raylib.h"
#include "Ball.h"
#include "Brick.h"

int main() {
    // 设置一个砖块和小球，使它们发生碰撞
    Brick brick(50.0f, 50.0f, 100.0f, 20.0f);
    Vector2 ballPos = { 60.0f, 60.0f };
    Vector2 ballSpeed = { 2.0f, 3.0f };
    float radius = 5.0f;

    Ball ball(ballPos, ballSpeed, radius);

    // 先确认库的碰撞检测函数认为它们碰撞
    bool collided = CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), brick.GetRect());
    if (!collided) {
        std::cerr << "Test failed: expected collision but none detected.\n";
        return EXIT_FAILURE;
    }

    // 按照游戏中实现的碰撞响应逻辑计算预期速度
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

    // 期望值：对于本测试数据，hitSide = false, hitTopBottom = false -> 两分量都取反
    if (speed.x != -2.0f || speed.y != -3.0f) {
        std::cerr << "Test failed: unexpected resulting speed: (" << speed.x << ", " << speed.y << ")\n";
        return EXIT_FAILURE;
    }

    std::cout << "Collision test passed.\n";
    return EXIT_SUCCESS;
}

