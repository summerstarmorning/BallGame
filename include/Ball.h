#ifndef BALL_H
#define BALL_H

#include "raylib.h"

class Ball {
// 位置
// 速度
// 半径(形状参数)
private:
    Vector2 position;
    Vector2 speed;
    float radius;
    
// 参数定义
// 运动
// 渲染
// 边界碰撞
public:
    Ball(Vector2 pos, Vector2 sp, float r);
    void Move();
    void Draw();
    void BounceEdge(int screenWidth, int screenHeight);
};

#endif