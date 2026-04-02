#include "Paddle.h"
//构造
//形状：左上角坐标与宽高
//相比brick同样是矩形不需要存活状态
Paddle::Paddle(float x, float y, float w, float h) {
    rect = { x, y, w, h };
}
//渲染
void Paddle::Draw() {
    DrawRectangleRec(rect, BLUE);
}
//往左移动
//传参每帧dpos
void Paddle::MoveLeft(float speed) {
    rect.x -= speed;
    // 像素墙设定
    if (rect.x < 5.0f) rect.x = 5.0f;
}

void Paddle::MoveRight(float speed) {
    rect.x += speed;
    
    float rightBound = GetScreenWidth() - 5.0f;
    if (rect.x + rect.width > rightBound) {
        rect.x = rightBound - rect.width;
    }
}