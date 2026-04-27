#include "Effects/PierceBallEffect.hpp"

#include <algorithm>
#include <cmath>

#include "GameWorld.hpp"

namespace game
{
void PierceBallEffect::apply(GameWorld& world)
{
    if (world.pierceCharges == nullptr)
    {
        return;
    }

    const int addedCharges = std::max(1, (int)std::lround(magnitude()));
    *world.pierceCharges += addedCharges;
}

void PierceBallEffect::update(GameWorld& world, float deltaSeconds)
{
    (void)deltaSeconds;

    if (world.pierceCharges == nullptr || *world.pierceCharges <= 0)
    {
        finish(world);
    }
}

void PierceBallEffect::expire(GameWorld& world)
{
    if (world.pierceCharges == nullptr)
    {
        return;
    }

    *world.pierceCharges = 0;
}
} // namespace game
