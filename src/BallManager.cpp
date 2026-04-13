#include "BallManager.hpp"

namespace game
{
void BallManager::addBall(const Ball& ball)
{
    balls_.push_back(ball);
}

void BallManager::update(float deltaSeconds, const Rect& playBounds)
{
    resetCollisionFlags();

    for (Ball& ball : balls_)
    {
        if (!ball.active)
        {
            continue;
        }

        ball.position.x += ball.velocity.x * speedMultiplier_ * deltaSeconds;
        ball.position.y += ball.velocity.y * speedMultiplier_ * deltaSeconds;
    }

    cullLostBalls(playBounds);
}

void BallManager::render() const
{
    // Rendering should be delegated to the engine adapter.
}

bool BallManager::consumeLifeLoss()
{
    if (!pendingLifeLoss_)
    {
        return false;
    }

    pendingLifeLoss_ = false;
    return true;
}

bool BallManager::hasActiveBalls() const noexcept
{
    for (const Ball& ball : balls_)
    {
        if (ball.active)
        {
            return true;
        }
    }

    return false;
}

void BallManager::setSpeedMultiplier(float multiplier) noexcept
{
    speedMultiplier_ = multiplier;
}

void BallManager::resetSpeedMultiplier() noexcept
{
    speedMultiplier_ = 1.0F;
}

void BallManager::resetCollisionFlags() noexcept
{
    for (Ball& ball : balls_)
    {
        ball.collisionResolvedThisFrame = false;
    }
}

void BallManager::cullLostBalls(const Rect& playBounds)
{
    for (Ball& ball : balls_)
    {
        if (ball.active && ball.position.y - ball.radius > playBounds.y + playBounds.height)
        {
            ball.active = false;
        }
    }

    if (!hasActiveBalls())
    {
        pendingLifeLoss_ = true;
    }
}
} // namespace game
