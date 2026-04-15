#pragma once

#include "Effects/PowerUpEffect.hpp"

namespace game
{
class PaddleSpeedEffect : public PowerUpEffect
{
public:
    using PowerUpEffect::PowerUpEffect;

    void apply(GameWorld& world) override;
    void expire(GameWorld& world) override;
};
} // namespace game
