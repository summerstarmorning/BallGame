#include "Config/PowerUpConfigLoader.hpp"

#include <fstream>
#include <stdexcept>

namespace game
{
std::string toString(PowerUpType type)
{
    switch (type)
    {
    case PowerUpType::ExpandPaddle:
        return "expand_paddle";
    case PowerUpType::MultiBall:
        return "multi_ball";
    case PowerUpType::SlowBall:
        return "slow_ball";
    }

    throw std::runtime_error("Unknown PowerUpType.");
}

PowerUpType powerUpTypeFromString(const std::string& value)
{
    if (value == "expand_paddle")
    {
        return PowerUpType::ExpandPaddle;
    }

    if (value == "multi_ball")
    {
        return PowerUpType::MultiBall;
    }

    if (value == "slow_ball")
    {
        return PowerUpType::SlowBall;
    }

    throw std::runtime_error("Unsupported power-up type: " + value);
}

PowerUpConfigSet PowerUpConfigLoader::loadFromFile(const std::filesystem::path& path) const
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Unable to open config file: " + path.string());
    }

    // JSON parsing should be wired to your chosen library here, for example nlohmann/json.
    // The interface is already isolated so the parser can be swapped without changing gameplay code.
    throw std::runtime_error("JSON parsing is not wired yet. Connect your JSON library in PowerUpConfigLoader.");
}
} // namespace game
