#include "Brick.h"

// 构造：矩形位置和大小，初始为激活状态
Brick::Brick(float x, float y, float w, float h) {
    rect = { x, y, w, h };
    active = true;
}

// 渲染：仅渲染处于激活状态的砖块
void Brick::Draw(Color c) {
    if (active) {
        DrawRectangleRounded(rect, 0.2f, 4, c);
        DrawRectangleRoundedLines(rect, 0.2f, 4, 2.0f, ColorAlpha(BLACK, 0.2f));
    }
}
