#include "Effects/SlowBallEffect.hpp"

#include <algorithm>

#include "Effects/PowerUpEffect.hpp"
#include "BallManager.hpp"
#include "GameWorld.hpp"

namespace game
{
void SlowBallEffect::apply(GameWorld& world)
{
    if (world.ballManager == nullptr)
    {
        return;
    }

    float bestMultiplier = std::clamp(config().magnitude, 1.0F, 3.5F);
    if (world.activeEffects != nullptr)
    {
        for (const std::unique_ptr<PowerUpEffect>& effect : *world.activeEffects)
        {
            if (!effect || effect->isFinished() || effect->type() != PowerUpType::SlowBall)
            {
                continue;
            }

            bestMultiplier = std::max(bestMultiplier, std::clamp(effect->magnitude(), 1.0F, 3.5F));
        }
    }

    world.ballManager->setSpeedMultiplier(bestMultiplier);
}

void SlowBallEffect::expire(GameWorld& world)
{
    if (world.ballManager == nullptr)
    {
        return;
    }

    float bestMultiplier = 1.0F;
    if (world.activeEffects != nullptr)
    {
        for (const std::unique_ptr<PowerUpEffect>& effect : *world.activeEffects)
        {
            if (!effect || effect.get() == this || effect->isFinished() || effect->type() != PowerUpType::SlowBall)
            {
                continue;
            }

            bestMultiplier = std::max(bestMultiplier, std::clamp(effect->magnitude(), 1.0F, 3.5F));
        }
    }

    if (bestMultiplier <= 1.001F)
    {
        world.ballManager->resetSpeedMultiplier();
        return;
    }

    world.ballManager->setSpeedMultiplier(bestMultiplier);
}
} // namespace game
