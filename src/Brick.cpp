#include "Brick.h"

// 构造：矩形位置和大小，初始为激活状态
Brick::Brick(float x, float y, float w, float h) {
    rect = { x, y, w, h };
    active = true;
}

// 渲染：仅渲染处于激活状态的砖块
void Brick::Draw() {
    if (active) {
        DrawRectangleRec(rect, GREEN);
    }
}
