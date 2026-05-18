#include "Level/LevelLoader.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "nlohmann_json.hpp"

using json = nlohmann::json;

namespace
{
game::BrickRecord parseBrickRecord(const json& item)
{
    game::BrickRecord brick;
    brick.x = item.value("x", brick.x);
    brick.y = item.value("y", brick.y);
    brick.width = std::max(20.0F, item.value("width", brick.width));
    brick.height = std::max(14.0F, item.value("height", brick.height));
    brick.shape = std::clamp(item.value("shape", brick.shape), 0, 5);
    brick.durability = std::max(1, item.value("durability", brick.durability));
    return brick;
}
} // namespace

namespace game
{
LevelData LevelLoader::loadFromFile(const std::string& path, int levelNumber, int screenWidth, int screenHeight) const
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        return buildFallbackLevel(levelNumber, screenWidth, screenHeight, "Missing level JSON file: " + path);
    }

    try
    {
        const json data = json::parse(input, nullptr, true, true);
        LevelData level;
        level.version = std::max(1, data.value("version", 1));
        level.name = data.value("name", "Level " + std::to_string(levelNumber));

        if (data.contains("game") && data["game"].is_object())
        {
            const json& gameConfig = data["game"];
            level.lives = std::max(1, gameConfig.value("lives", level.lives));
        }

        if (data.contains("ball") && data["ball"].is_object())
        {
            const json& ball = data["ball"];
            level.ballStart.x = ball.value("startX", level.ballStart.x);
            level.ballStart.y = ball.value("startY", level.ballStart.y);
            level.ballSpeed.x = ball.value("speedX", level.ballSpeed.x);
            level.ballSpeed.y = ball.value("speedY", level.ballSpeed.y);
            level.ballRadius = std::max(10.0F, ball.value("radius", level.ballRadius));
        }

        if (data.contains("paddle") && data["paddle"].is_object())
        {
            const json& paddle = data["paddle"];
            level.paddleStart.x = paddle.value("startX", level.paddleStart.x);
            level.paddleStart.y = paddle.value("startY", level.paddleStart.y);
            level.paddleStart.width = std::max(80.0F, paddle.value("width", level.paddleStart.width));
            level.paddleStart.height = std::max(16.0F, paddle.value("height", level.paddleStart.height));
        }

        if (!data.contains("bricks") || !data["bricks"].is_array())
        {
            return buildFallbackLevel(levelNumber, screenWidth, screenHeight, "Level JSON has no valid bricks array: " + path);
        }

        for (const json& item : data["bricks"])
        {
            if (!item.is_object())
            {
                continue;
            }
            level.bricks.push_back(parseBrickRecord(item));
        }

        if (level.bricks.empty())
        {
            return buildFallbackLevel(levelNumber, screenWidth, screenHeight, "Level JSON bricks array is empty: " + path);
        }

        return level;
    }
    catch (const json::parse_error& error)
    {
        return buildFallbackLevel(levelNumber, screenWidth, screenHeight, std::string("Level JSON parse error: ") + error.what());
    }
    catch (const std::exception& error)
    {
        return buildFallbackLevel(levelNumber, screenWidth, screenHeight, std::string("Level JSON load failed: ") + error.what());
    }
}

bool LevelLoader::saveToFile(const std::string& path, const LevelData& level, std::string* errorMessage) const
{
    namespace fs = std::filesystem;

    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);

    json data;
    data["version"] = level.version;
    data["name"] = level.name;
    data["game"] = {{"lives", level.lives}};
    data["ball"] = {
        {"startX", level.ballStart.x},
        {"startY", level.ballStart.y},
        {"speedX", level.ballSpeed.x},
        {"speedY", level.ballSpeed.y},
        {"radius", level.ballRadius},
    };
    data["paddle"] = {
        {"startX", level.paddleStart.x},
        {"startY", level.paddleStart.y},
        {"width", level.paddleStart.width},
        {"height", level.paddleStart.height},
    };
    data["bricks"] = json::array();
    for (const BrickRecord& brick : level.bricks)
    {
        data["bricks"].push_back({
            {"x", brick.x},
            {"y", brick.y},
            {"width", brick.width},
            {"height", brick.height},
            {"shape", brick.shape},
            {"durability", brick.durability},
        });
    }

    std::ofstream output(path);
    if (!output.is_open())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = "Failed to open level file for writing: " + path;
        }
        return false;
    }

    output << data.dump(4);
    return true;
}

LevelData LevelLoader::buildFallbackLevel(int levelNumber, int screenWidth, int screenHeight, const std::string& reason) const
{
    LevelData level;
    level.name = "Fallback " + std::to_string(levelNumber);
    level.usedFallback = true;
    level.warning = reason;
    level.ballStart = Vector2 {(float)screenWidth * 0.5F, (float)screenHeight * 0.45F};
    level.ballSpeed = Vector2 {4.8F + (float)levelNumber * 0.7F, 4.8F + (float)levelNumber * 0.5F};
    level.ballRadius = 15.0F;
    level.paddleStart = Rectangle {
        (float)screenWidth * 0.5F - 70.0F,
        (float)screenHeight - 76.0F,
        140.0F,
        24.0F,
    };
    level.lives = 3;

    const int rows = 4 + std::min(levelNumber, 3);
    const int columns = 8 + std::min(levelNumber * 2, 8);
    const float width = 88.0F;
    const float height = 26.0F;
    const float gapX = 10.0F;
    const float gapY = 12.0F;
    const float startX = std::max(44.0F, ((float)screenWidth - ((float)columns * width + (float)(columns - 1) * gapX)) * 0.5F);
    const float startY = 142.0F;

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            BrickRecord brick;
            brick.x = startX + (float)column * (width + gapX);
            brick.y = startY + (float)row * (height + gapY);
            brick.width = width;
            brick.height = height;
            brick.shape = (row + column + levelNumber) % 6;
            brick.durability = 1 + ((row + levelNumber) % 3 == 0 ? 1 : 0) + ((column + row) % 7 == 0 ? 1 : 0);
            level.bricks.push_back(brick);
        }
    }

    return level;
}
} // namespace game
