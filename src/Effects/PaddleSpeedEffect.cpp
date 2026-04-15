#include "Effects/PaddleSpeedEffect.hpp"

#include <algorithm>

#include "Effects/PowerUpEffect.hpp"
#include "GameWorld.hpp"

namespace game
{
void PaddleSpeedEffect::apply(GameWorld& world)
{
    if (world.paddleSpeedMultiplier == nullptr)
    {
        return;
    }

    float bestMultiplier = config().magnitude;
    if (world.activeEffects != nullptr)
    {
        for (const std::unique_ptr<PowerUpEffect>& effect : *world.activeEffects)
        {
            if (!effect || effect->isFinished() || effect->type() != PowerUpType::PaddleSpeed)
            {
                continue;
            }

            bestMultiplier = std::max(bestMultiplier, effect->magnitude());
        }
    }

    *world.paddleSpeedMultiplier = std::max(1.0F, bestMultiplier);
}

void PaddleSpeedEffect::expire(GameWorld& world)
{
    if (world.paddleSpeedMultiplier == nullptr)
    {
        return;
    }

    float bestMultiplier = 1.0F;
    if (world.activeEffects != nullptr)
    {
        for (const std::unique_ptr<PowerUpEffect>& effect : *world.activeEffects)
        {
            if (!effect || effect.get() == this || effect->isFinished() || effect->type() != PowerUpType::PaddleSpeed)
            {
                continue;
            }

            bestMultiplier = std::max(bestMultiplier, effect->magnitude());
        }
    }

    *world.paddleSpeedMultiplier = std::max(1.0F, bestMultiplier);
}
} // namespace game
