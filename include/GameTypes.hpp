#pragma once

#include <cstddef>

namespace game
{
struct Vec2
{
    float x {0.0F};
    float y {0.0F};
};

struct Rect
{
    float x {0.0F};
    float y {0.0F};
    float width {0.0F};
    float height {0.0F};

    bool intersects(const Rect& other) const noexcept
    {
        return x < other.x + other.width
            && x + width > other.x
            && y < other.y + other.height
            && y + height > other.y;
    }
};

struct Color
{
    unsigned char r {255};
    unsigned char g {255};
    unsigned char b {255};
    unsigned char a {255};
};
} // namespace game
