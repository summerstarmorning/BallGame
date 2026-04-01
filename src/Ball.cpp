#include "Ball.h"

Ball::Ball(Vector2 pos, Vector2 sp, float r) {
    position = pos;
    speed = sp;
    radius = r;
}

void Ball::Move() {
    position.x += speed.x;
    position.y += speed.y;
}

void Ball::Draw() {
    DrawCircleV(position, radius, RED);
}

void Ball::BounceEdge(int screenWidth, int screenHeight) {
    if (position.x - radius <= 0 || position.x + radius >= screenWidth) {
        speed.x *= -1;
    }
    if (position.y - radius <= 0) {
        speed.y *= -1;
    }
    if (position.y + radius >= screenHeight) {
        speed.y *= -1;
    }
}