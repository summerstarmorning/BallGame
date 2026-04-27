#pragma once

#include <string>
#include <unordered_map>

namespace game
{
enum class PowerUpType
{
    ExpandPaddle,
    MultiBall,
    SlowBall,
    PaddleSpeed,
    PierceBall
};

struct PowerUpConfig
{
    PowerUpType type {};
    float dropChance {0.0F};
    float durationSeconds {0.0F};
    float magnitude {1.0F};
    bool permanent {false};
};

struct ParticleConfig
{
    std::size_t maxCount {512U};
    std::size_t brickBurstCount {18U};
    std::size_t powerUpTrailCount {4U};
};

struct PowerUpConfigSet
{
    std::unordered_map<PowerUpType, PowerUpConfig> powerUps {};
    ParticleConfig particles {};
};

std::string toString(PowerUpType type);
PowerUpType powerUpTypeFromString(const std::string& value);
} // namespace game
