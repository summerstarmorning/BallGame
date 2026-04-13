#pragma once

#include <memory>

#include "Config/PowerUpConfig.hpp"

namespace game
{
class PowerUpEffect;

class PowerUpFactory
{
public:
    std::unique_ptr<PowerUpEffect> create(const PowerUpConfig& config) const;
};
} // namespace game
