#pragma once

#include <algorithm>
#include <array>

#include "Config/PowerUpConfig.hpp"
#include "GameTypes.hpp"
#include "raylib.h"

namespace game_style
{
inline const std::array<Color, 8>& colorPalette()
{
    static const std::array<Color, 8> palette {
        RED,
        BLUE,
        GREEN,
        ORANGE,
        PURPLE,
        MAROON,
        DARKBLUE,
        DARKGREEN,
    };
    return palette;
}

inline const std::array<const char*, 2>& levelConfigs()
{
    static const std::array<const char*, 2> levels {"config.json", "config2.json"};
    return levels;
}

inline game::Vec2 toGameVec(const Vector2& value)
{
    return game::Vec2 {value.x, value.y};
}

inline Vector2 toRayVec(const game::Vec2& value)
{
    return Vector2 {value.x, value.y};
}

inline game::Rect toGameRect(const Rectangle& value)
{
    return game::Rect {value.x, value.y, value.width, value.height};
}

inline Rectangle toRayRect(const game::Rect& value)
{
    return Rectangle {value.x, value.y, value.width, value.height};
}

inline Color toRayColor(const game::Color& value)
{
    return Color {value.r, value.g, value.b, value.a};
}

inline Color mixColor(Color from, Color to, float t)
{
    const float factor = std::clamp(t, 0.0F, 1.0F);
    Color mixed {};
    mixed.r = (unsigned char)((1.0F - factor) * (float)from.r + factor * (float)to.r);
    mixed.g = (unsigned char)((1.0F - factor) * (float)from.g + factor * (float)to.g);
    mixed.b = (unsigned char)((1.0F - factor) * (float)from.b + factor * (float)to.b);
    mixed.a = (unsigned char)((1.0F - factor) * (float)from.a + factor * (float)to.a);
    return mixed;
}

inline const char* effectLabelEn(game::PowerUpType type)
{
    switch (type)
    {
    case game::PowerUpType::ExpandPaddle:
        return "MEGA PADDLE";
    case game::PowerUpType::MultiBall:
        return "MULTI SPLIT";
    case game::PowerUpType::SlowBall:
        return "BALL BOOST";
    case game::PowerUpType::PaddleSpeed:
        return "PADDLE BOOST";
    case game::PowerUpType::PierceBall:
        return "PIERCE SHOT";
    }

    return "EFFECT";
}

inline const char* effectLabelZh(game::PowerUpType type)
{
    switch (type)
    {
    case game::PowerUpType::ExpandPaddle:
        return u8"\u8d85\u7ea7\u957f\u677f";
    case game::PowerUpType::MultiBall:
        return u8"\u591a\u7403\u5206\u88c2";
    case game::PowerUpType::SlowBall:
        return u8"\u7403\u901f\u63d0\u5347";
    case game::PowerUpType::PaddleSpeed:
        return u8"\u6321\u677f\u52a0\u901f";
    case game::PowerUpType::PierceBall:
        return u8"\u8d2f\u7a7f\u5f39";
    }

    return u8"\u9053\u5177\u6548\u679c";
}

inline const char* powerUpGlyph(game::PowerUpType type)
{
    switch (type)
    {
    case game::PowerUpType::ExpandPaddle:
        return "E";
    case game::PowerUpType::MultiBall:
        return "M";
    case game::PowerUpType::SlowBall:
        return "S";
    case game::PowerUpType::PaddleSpeed:
        return "B";
    case game::PowerUpType::PierceBall:
        return "P";
    }

    return "?";
}

inline Color powerUpColor(game::PowerUpType type)
{
    switch (type)
    {
    case game::PowerUpType::ExpandPaddle:
        return SKYBLUE;
    case game::PowerUpType::MultiBall:
        return GOLD;
    case game::PowerUpType::SlowBall:
        return ORANGE;
    case game::PowerUpType::PaddleSpeed:
        return Color {255, 108, 232, 255};
    case game::PowerUpType::PierceBall:
        return Color {255, 92, 92, 255};
    }

    return GRAY;
}

inline Rectangle pauseButtonRect(int screenWidth, int buttonSize)
{
    return Rectangle {(float)screenWidth - 72.0F, 24.0F, (float)buttonSize, (float)buttonSize};
}
} // namespace game_style
