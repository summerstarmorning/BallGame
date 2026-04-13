#pragma once

#include "GameTypes.hpp"

namespace game
{
class Paddle
{
public:
    explicit Paddle(Rect bounds) : bounds_(bounds), baseWidth_(bounds.width) {}

    const Rect& bounds() const noexcept { return bounds_; }
    void setBounds(const Rect& bounds) noexcept { bounds_ = bounds; }

    void setWidthMultiplier(float multiplier) noexcept
    {
        bounds_.width = baseWidth_ * multiplier;
    }

    void resetWidth() noexcept
    {
        bounds_.width = baseWidth_;
    }

private:
    Rect bounds_ {};
    float baseWidth_ {0.0F};
};
} // namespace game
