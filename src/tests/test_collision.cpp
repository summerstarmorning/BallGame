#include <iostream>
#include <cstdlib>
#include "raylib.h"
#include "Ball.h"
#include "Brick.h"

void TestSideCollision() {
    Brick brick(50.0f, 50.0f, 100.0f, 20.0f);
    // Ball moving right hit the left side of brick
    Ball ball({ 45.0f, 60.0f }, { 3.0f, 0.0f }, 10.0f);
    
    bool collided = ball.CheckBrickCollision(brick.GetRect());
    if (!collided || ball.GetSpeed().x != -3.0f) {
        std::cerr << "TestSideCollision failed! Spd x = " << ball.GetSpeed().x << "\n";
        exit(EXIT_FAILURE);
    }
}

void TestTopCollision() {
    Brick brick(50.0f, 50.0f, 100.0f, 20.0f);
    // Ball moving down hit the top side of brick
    Ball ball({ 100.0f, 45.0f }, { 0.0f, 3.0f }, 10.0f);
    
    bool collided = ball.CheckBrickCollision(brick.GetRect());
    if (!collided || ball.GetSpeed().y != -3.0f) {
        std::cerr << "TestTopCollision failed! Spd y = " << ball.GetSpeed().y << "\n";
        exit(EXIT_FAILURE);
    }
}

int main() {
    TestSideCollision();
    TestTopCollision();

    std::cout << "All collision tests passed.\n";
    return EXIT_SUCCESS;
}

