#include "PowerUps/PowerUpFactory.hpp"

#include <stdexcept>
#include <utility>

#include "Effects/ExpandPaddleEffect.hpp"
#include "Effects/MultiBallEffect.hpp"
#include "Effects/PaddleSpeedEffect.hpp"
#include "Effects/PierceBallEffect.hpp"
#include "Effects/PowerUpEffect.hpp"
#include "Effects/SlowBallEffect.hpp"

namespace game
{
PowerUpFactory::PowerUpFactory()
{
    registerCreator(
        PowerUpType::ExpandPaddle,
        [](const PowerUpConfig& config)
        {
            return std::make_unique<ExpandPaddleEffect>(config);
        });
    registerCreator(
        PowerUpType::MultiBall,
        [](const PowerUpConfig& config)
        {
            return std::make_unique<MultiBallEffect>(config);
        });
    registerCreator(
        PowerUpType::SlowBall,
        [](const PowerUpConfig& config)
        {
            return std::make_unique<SlowBallEffect>(config);
        });
    registerCreator(
        PowerUpType::PaddleSpeed,
        [](const PowerUpConfig& config)
        {
            return std::make_unique<PaddleSpeedEffect>(config);
        });
    registerCreator(
        PowerUpType::PierceBall,
        [](const PowerUpConfig& config)
        {
            return std::make_unique<PierceBallEffect>(config);
        });
}

void PowerUpFactory::registerCreator(PowerUpType type, Creator creator)
{
    creators_[type] = std::move(creator);
}

std::unique_ptr<PowerUpEffect> PowerUpFactory::create(const PowerUpConfig& config) const
{
    const auto it = creators_.find(config.type);
    if (it == creators_.end())
    {
        throw std::runtime_error("Missing factory registration for power-up.");
    }

    return it->second(config);
}
} // namespace game
