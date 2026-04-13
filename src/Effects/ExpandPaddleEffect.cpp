#include "Effects/ExpandPaddleEffect.hpp"

#include "GameWorld.hpp"
#include "Paddle.hpp"

namespace game
{
void ExpandPaddleEffect::apply(GameWorld& world)
{
    if (world.paddle != nullptr)
    {
        world.paddle->setWidthMultiplier(config().magnitude);
    }
}

void ExpandPaddleEffect::expire(GameWorld& world)
{
    if (world.paddle != nullptr)
    {
        world.paddle->resetWidth();
    }
}
} // namespace game
