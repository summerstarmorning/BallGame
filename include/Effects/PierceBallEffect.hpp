#pragma once

#include "Effects/PowerUpEffect.hpp"

namespace game
{
class PierceBallEffect : public PowerUpEffect
{
public:
    using PowerUpEffect::PowerUpEffect;

    void apply(GameWorld& world) override;
    void update(GameWorld& world, float deltaSeconds) override;
    void expire(GameWorld& world) override;
    bool shouldReapplyOnLevelTransition() const noexcept override { return false; }
};
} // namespace game
