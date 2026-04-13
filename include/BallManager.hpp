#pragma once

#include <vector>

#include "GameTypes.hpp"

namespace game
{
struct Ball
{
    Vec2 position {};
    Vec2 velocity {};
    float radius {8.0F};
    bool active {true};
    bool collisionResolvedThisFrame {false};
};

class BallManager
{
public:
    void addBall(const Ball& ball);
    void update(float deltaSeconds, const Rect& playBounds);
    void render() const;

    bool consumeLifeLoss();
    bool hasActiveBalls() const noexcept;
    void setSpeedMultiplier(float multiplier) noexcept;
    void resetSpeedMultiplier() noexcept;

    std::vector<Ball>& balls() noexcept { return balls_; }
    const std::vector<Ball>& balls() const noexcept { return balls_; }

private:
    void resetCollisionFlags() noexcept;
    void cullLostBalls(const Rect& playBounds);

    std::vector<Ball> balls_ {};
    float speedMultiplier_ {1.0F};
    bool pendingLifeLoss_ {false};
};
} // namespace game
