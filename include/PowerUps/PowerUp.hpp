#pragma once

#include "Config/PowerUpConfig.hpp"
#include "GameTypes.hpp"

namespace game
{
class GameWorld;
class Paddle;

class PowerUp
{
public:
    PowerUp(PowerUpType type, Vec2 position, Vec2 velocity);

    void update(float deltaSeconds);
    void render() const;
    bool checkPickup(const Paddle& paddle) const noexcept;
    void markCollected() noexcept;
    bool isCollected() const noexcept { return collected_; }
    bool isOffScreen(float heightLimit) const noexcept;

    PowerUpType type() const noexcept { return type_; }
    const Vec2& position() const noexcept { return position_; }
    Rect bounds() const noexcept;

private:
    PowerUpType type_ {};
    Vec2 position_ {};
    Vec2 velocity_ {};
    Vec2 size_ {24.0F, 12.0F};
    bool collected_ {false};
};
} // namespace game
