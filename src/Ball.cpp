#include "Ball.h"

//构造(位置,速度，半径)
Ball::Ball(Vector2 pos, Vector2 sp, float r)
    : position(pos), speed(sp), radius(r) {}

//pos(t+dt)=pos(t)+vdt
void Ball::Move() {
    position.x += speed.x;
    position.y += speed.y;
}
//渲染
void Ball::Draw() {
    DrawCircleV(position, radius, RED);
}
//边缘碰撞
//传入屏幕宽高
void Ball::BounceEdge(int screenWidth, int screenHeight) {
    //圆心坐标有效边界
    const float left = radius;
    const float right = static_cast<float>(screenWidth) - radius;
    const float top = radius;
    const float bottom = static_cast<float>(screenHeight) - radius;
	//碰撞检测
    //会有一个一顿一顿的位置纠正以及速度瞬间反向
    //x,y正交线性处理
    if (position.x < left) {
        position.x = left;
        if (speed.x < 0.0f) speed.x = -speed.x;
    } else if (position.x > right) {
        position.x = right;
        if (speed.x > 0.0f) speed.x = -speed.x;
    }

    if (position.y < top) {
        position.y = top;
        if (speed.y < 0.0f) speed.y = -speed.y;
    } else if (position.y > bottom) {
        position.y = bottom;
        if (speed.y > 0.0f) speed.y = -speed.y;
    }
}