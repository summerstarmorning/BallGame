#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"

class Brick {
// 形状
// 存活状态
private:
    Rectangle rect;
    bool active;
    
// 构造参数：形状
// 渲染
// 是否存活状态接口
// 设置存活状态
public:
    Brick(float x, float y, float w, float h);
    void Draw();
    bool IsActive() const { return active; }
    void SetActive(bool a) { active = a; }
    
    Rectangle GetRect() const { return rect; }
};

#endif