#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace game
{
struct BrickRecord
{
    float x {0.0F};
    float y {0.0F};
    float width {96.0F};
    float height {28.0F};
    int shape {1};
    int durability {1};
};

struct LevelData
{
    int version {1};
    std::string name {"Fallback Level"};
    int lives {3};
    Vector2 ballStart {(float)800 / 2.0F, (float)600 / 2.0F};
    Vector2 ballSpeed {4.0F, 4.0F};
    float ballRadius {15.0F};
    Rectangle paddleStart {320.0F, 700.0F, 150.0F, 25.0F};
    std::vector<BrickRecord> bricks;
    bool usedFallback {false};
    std::string warning;
};
} // namespace game
