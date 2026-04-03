#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"

class Paddle {
// 形状
private:
    Rectangle rect;
// 参数定义
// 渲染
// 移动
public:
    Paddle(float x, float y, float w, float h);
    void Draw(Color c = BLUE);
    void MoveLeft(float speed);
    void MoveRight(float speed);
    
    Rectangle GetRect() const { return rect; }
};

#endif