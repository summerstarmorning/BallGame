#include "PowerUps/PowerUpFactory.hpp"

#include <stdexcept>

#include "Effects/ExpandPaddleEffect.hpp"
#include "Effects/MultiBallEffect.hpp"
#include "Effects/PowerUpEffect.hpp"
#include "Effects/SlowBallEffect.hpp"

namespace game
{
std::unique_ptr<PowerUpEffect> PowerUpFactory::create(const PowerUpConfig& config) const
{
    switch (config.type)
    {
    case PowerUpType::ExpandPaddle:
        return std::make_unique<ExpandPaddleEffect>(config);
    case PowerUpType::MultiBall:
        return std::make_unique<MultiBallEffect>(config);
    case PowerUpType::SlowBall:
        return std::make_unique<SlowBallEffect>(config);
    }

    throw std::runtime_error("Missing factory registration for power-up.");
}
} // namespace game
