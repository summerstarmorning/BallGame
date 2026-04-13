#include "Effects/PowerUpEffect.hpp"

#include "GameWorld.hpp"

namespace game
{
void PowerUpEffect::update(GameWorld& world, float deltaSeconds)
{
    if (config().permanent || finished_)
    {
        return;
    }

    elapsedSeconds_ += deltaSeconds;
    if (elapsedSeconds_ >= config().durationSeconds)
    {
        finish(world);
    }
}

bool PowerUpEffect::isFinished() const noexcept
{
    return finished_;
}

void PowerUpEffect::finish(GameWorld& world)
{
    if (expired_)
    {
        return;
    }

    expire(world);
    expired_ = true;
    finished_ = true;
}
} // namespace game
