#include "Effects/SlowBallEffect.hpp"

#include "BallManager.hpp"
#include "GameWorld.hpp"

namespace game
{
void SlowBallEffect::apply(GameWorld& world)
{
    if (world.ballManager != nullptr)
    {
        world.ballManager->setSpeedMultiplier(config().magnitude);
    }
}

void SlowBallEffect::expire(GameWorld& world)
{
    if (world.ballManager != nullptr)
    {
        world.ballManager->resetSpeedMultiplier();
    }
}
} // namespace game
