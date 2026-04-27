#include "Config/PowerUpConfigLoader.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

#include "nlohmann_json.hpp"

namespace game
{
using json = nlohmann::json;

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
    case PowerUpType::PaddleSpeed:
        return "paddle_speed";
    case PowerUpType::PierceBall:
        return "pierce_ball";
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

    if (value == "paddle_speed")
    {
        return PowerUpType::PaddleSpeed;
    }

    if (value == "pierce_ball")
    {
        return PowerUpType::PierceBall;
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

    json document = json::parse(input, nullptr, true, true);

    PowerUpConfigSet configSet {};

    if (document.contains("particles") && document["particles"].is_object())
    {
        const json& particles = document["particles"];
        configSet.particles.maxCount = particles.value("maxCount", configSet.particles.maxCount);
        configSet.particles.brickBurstCount = particles.value("brickBurstCount", configSet.particles.brickBurstCount);
        configSet.particles.powerUpTrailCount = particles.value("powerUpTrailCount", configSet.particles.powerUpTrailCount);
    }

    if (!document.contains("powerups") || !document["powerups"].is_array())
    {
        throw std::runtime_error("Invalid power-up config: missing array field 'powerups'.");
    }

    for (const json& item : document["powerups"])
    {
        if (!item.is_object())
        {
            continue;
        }

        const std::string typeName = item.value("type", "");
        if (typeName.empty())
        {
            continue;
        }

        PowerUpConfig config {};
        config.type = powerUpTypeFromString(typeName);
        config.dropChance = std::clamp(item.value("dropChance", 0.0F), 0.0F, 1.0F);
        config.durationSeconds = std::max(0.0F, item.value("durationSeconds", 0.0F));
        config.magnitude = std::max(0.0F, item.value("magnitude", 1.0F));
        config.permanent = item.value("permanent", false);

        configSet.powerUps[config.type] = config;
    }

    if (configSet.powerUps.empty())
    {
        throw std::runtime_error("Power-up config array is empty.");
    }

    return configSet;
}
} // namespace game
