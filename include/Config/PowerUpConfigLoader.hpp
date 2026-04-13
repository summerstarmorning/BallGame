#pragma once

#include <filesystem>

#include "Config/PowerUpConfig.hpp"

namespace game
{
class PowerUpConfigLoader
{
public:
    PowerUpConfigSet loadFromFile(const std::filesystem::path& path) const;
};
} // namespace game
