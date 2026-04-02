#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

int main() {
int screenWidth = 800;
int screenHeight = 600;
InitWindow(screenWidth, screenHeight, "Break all the bricks with the ball");

Ball ball({400.0f, 300.0f}, {2.0f, 2.0f}, 10.0f);
Paddle paddle(350.0f, 550.0f, 100.0f, 20.0f);

std::vector<Brick> bricks;
float brickWidth = 100.0f;
float brickHeight = 30.0f;

for (int i = 0; i < 8; i++) {
bricks.emplace_back(50.0f + i * 120.0f, 100.0f, brickWidth, brickHeight);
}

SetTargetFPS(60);

bool exitWindowRequest = false;

while (!exitWindowRequest && !WindowShouldClose()) {
if (IsKeyPressed(KEY_Q)) {
exitWindowRequest = true;
}

ball.Move();
ball.BounceEdge(screenWidth, screenHeight);

if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {
Vector2 speed = ball.GetSpeed();
if (speed.y > 0) {
speed.y = -speed.y;
float paddleCenter = paddle.GetRect().x + paddle.GetRect().width / 2.0f;
float hitOffset = ball.GetPosition().x - paddleCenter;
speed.x += hitOffset * 0.05f; 
ball.SetSpeed(speed);
}
}

for (auto& brick : bricks) {
if (brick.IsActive() && CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), brick.GetRect())) {
brick.SetActive(false);

Vector2 speed = ball.GetSpeed();
Vector2 pos = ball.GetPosition();
Rectangle rect = brick.GetRect();

bool hitSide = (pos.x < rect.x) || (pos.x > rect.x + rect.width);
bool hitTopBottom = (pos.y < rect.y) || (pos.y > rect.y + rect.height);

if (hitSide && !hitTopBottom) {
speed.x = -speed.x;
} else if (!hitSide && hitTopBottom) {
speed.y = -speed.y;
} else {
speed.x = -speed.x;
speed.y = -speed.y;
}

ball.SetSpeed(speed);
break;
}
}

if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5.0f);
if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5.0f);

BeginDrawing();
ClearBackground(RAYWHITE);

DrawRectangle(0, 0, 5, screenHeight, GRAY);
DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
DrawRectangle(0, 0, screenWidth, 5, GRAY);
DrawRectangle(0, screenHeight - 5, screenWidth, 5, GRAY);

ball.Draw();
paddle.Draw();
for (auto& brick : bricks) {
brick.Draw();
}

EndDrawing();
}

CloseWindow();
return 0;
}