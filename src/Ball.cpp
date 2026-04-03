#include "Ball.h"

// 构造函数 (位置, 速度, 半径)
Ball::Ball(Vector2 pos, Vector2 sp, float r)
    : position(pos), speed(sp), radius(r) {}

// 更新位置: pos(t+dt) = pos(t) + v * dt
void Ball::Move() {
    position.x += speed.x;
    position.y += speed.y;
}

// 渲染小球
void Ball::Draw(Color c) {
    DrawCircleV(position, radius, c);
}

// 边缘碰撞检测与响应
// 参数: 屏幕宽高
void Ball::BounceEdge(int screenWidth, int screenHeight) {
    // 圆心允许的有效边界
    const float left = radius;
    const float right = static_cast<float>(screenWidth) - radius;
    const float top = radius;
    const float bottom = static_cast<float>(screenHeight) - radius;

    // 碰撞检测及速度修正
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
    }
    // 不再处理底部边界，当球掉出屏幕由主逻辑处理生命值丢失
}

bool Ball::CheckBrickCollision(Rectangle rect) {
    if (CheckCollisionCircleRec(position, radius, rect)) {
        bool hitSide = (position.x < rect.x) || (position.x > rect.x + rect.width);
        bool hitTopBottom = (position.y < rect.y) || (position.y > rect.y + rect.height);

        if (hitSide && !hitTopBottom) speed.x = -speed.x;
        else if (!hitSide && hitTopBottom) speed.y = -speed.y;
        else { speed.x = -speed.x; speed.y = -speed.y; }

        return true;
    }
    return false;
}