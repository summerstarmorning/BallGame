#include "Paddle.h"

// 构造：矩形由左上角坐标和宽高定义
Paddle::Paddle(float x, float y, float w, float h) {
    rect = { x, y, w, h };
}

// 渲染挡板
void Paddle::Draw(Color c) {
    DrawRectangleRounded(rect, 0.5f, 4, c);
}

// 往左移动，每帧移动量由 speed 指定
void Paddle::MoveLeft(float speed) {
    rect.x -= speed;
    // 左侧碰撞墙
    if (rect.x < 5.0f) rect.x = 5.0f;
}

// 往右移动
void Paddle::MoveRight(float speed) {
    rect.x += speed;

    float rightBound = GetScreenWidth() - 5.0f;
    if (rect.x + rect.width > rightBound) {
        rect.x = rightBound - rect.width;
    }
}

void Paddle::MoveUp(float speed, float minY) {
    rect.y -= speed;
    if (rect.y < minY) {
        rect.y = minY;
    }
}

void Paddle::MoveDown(float speed, float maxY) {
    rect.y += speed;
    if (rect.y + rect.height > maxY) {
        rect.y = maxY - rect.height;
    }
}

void Paddle::SetRect(const Rectangle& newRect) {
    rect = newRect;
}
