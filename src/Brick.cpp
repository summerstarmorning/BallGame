#include "Brick.h"
//构造
//形状：左上角坐标与宽高
//存活状态：True
Brick::Brick(float x, float y, float w, float h) {
    rect = { x, y, w, h };
    active = true;
}
//渲染
void Brick::Draw() {
    if (active) {
        DrawRectangleRec(rect, GREEN);
    }
}
