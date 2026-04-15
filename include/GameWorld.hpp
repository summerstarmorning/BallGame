#pragma once

#include <memory>
#include <vector>

#include "BallManager.hpp"
#include "Particles/ParticleSystem.hpp"

namespace game
{
class Paddle;
class PowerUpEffect;

class GameWorld
{
public:
    Paddle* paddle {nullptr};
    BallManager* ballManager {nullptr};
    ParticleSystem* particleSystem {nullptr};
    std::vector<std::unique_ptr<PowerUpEffect>>* activeEffects {nullptr};
    float* paddleSpeedMultiplier {nullptr};
};
} // namespace game
