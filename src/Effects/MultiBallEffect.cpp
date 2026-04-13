#include "Effects/MultiBallEffect.hpp"

#include "BallManager.hpp"
#include "GameWorld.hpp"

namespace game
{
void MultiBallEffect::apply(GameWorld& world)
{
    if (world.ballManager == nullptr || world.ballManager->balls().empty())
    {
        return;
    }

    const Ball source = world.ballManager->balls().front();
    Ball clone = source;
    clone.velocity.x = -clone.velocity.x;
    world.ballManager->addBall(clone);
}

void MultiBallEffect::expire(GameWorld& world)
{
    (void)world;
}
} // namespace game
