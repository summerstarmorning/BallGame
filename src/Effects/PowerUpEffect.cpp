#include "Effects/PowerUpEffect.hpp"

#include <algorithm>

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
    if (elapsedSeconds_ >= totalDurationSeconds())
    {
        finish(world);
    }
}

bool PowerUpEffect::isFinished() const noexcept
{
    return finished_;
}

void PowerUpEffect::onLevelStarted(GameWorld& world)
{
    if (finished_ || !shouldReapplyOnLevelTransition())
    {
        return;
    }

    apply(world);
}

float PowerUpEffect::remainingSeconds() const noexcept
{
    if (config().permanent)
    {
        return 0.0F;
    }

    return std::max(0.0F, totalDurationSeconds() - elapsedSeconds_);
}

float PowerUpEffect::totalDurationSeconds() const noexcept
{
    return config().durationSeconds + bonusDurationSeconds_;
}

void PowerUpEffect::extendDuration(float extraSeconds) noexcept
{
    if (config().permanent || extraSeconds <= 0.0F)
    {
        return;
    }

    bonusDurationSeconds_ += extraSeconds;
}

void PowerUpEffect::forceExpire(GameWorld& world)
{
    if (finished_)
    {
        return;
    }

    finish(world);
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
