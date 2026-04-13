#include "PowerUps/PowerUp.hpp"

#include "Paddle.hpp"

namespace game
{
PowerUp::PowerUp(PowerUpType type, Vec2 position, Vec2 velocity)
    : type_(type), position_(position), velocity_(velocity)
{
}

void PowerUp::update(float deltaSeconds)
{
    position_.x += velocity_.x * deltaSeconds;
    position_.y += velocity_.y * deltaSeconds;
}

void PowerUp::render() const
{
    // Rendering should be delegated to the engine adapter.
}

bool PowerUp::checkPickup(const Paddle& paddle) const noexcept
{
    return bounds().intersects(paddle.bounds());
}

void PowerUp::markCollected() noexcept
{
    collected_ = true;
}

bool PowerUp::isOffScreen(float heightLimit) const noexcept
{
    return position_.y > heightLimit;
}

Rect PowerUp::bounds() const noexcept
{
    return Rect {position_.x, position_.y, size_.x, size_.y};
}
} // namespace game
