#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "Config/PowerUpConfig.hpp"

namespace game
{
class PowerUpEffect;

class PowerUpFactory
{
public:
    using Creator = std::function<std::unique_ptr<PowerUpEffect>(const PowerUpConfig&)>;

    PowerUpFactory();

    void registerCreator(PowerUpType type, Creator creator);
    std::unique_ptr<PowerUpEffect> create(const PowerUpConfig& config) const;

private:
    std::unordered_map<PowerUpType, Creator> creators_ {};
};
} // namespace game
