#pragma once

#include <string>

#include "Level/LevelData.hpp"

namespace game
{
class LevelLoader
{
public:
    LevelData loadFromFile(const std::string& path, int levelNumber, int screenWidth, int screenHeight) const;
    bool saveToFile(const std::string& path, const LevelData& level, std::string* errorMessage = nullptr) const;

private:
    LevelData buildFallbackLevel(int levelNumber, int screenWidth, int screenHeight, const std::string& reason) const;
};
} // namespace game
