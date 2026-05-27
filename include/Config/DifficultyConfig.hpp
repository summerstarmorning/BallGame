#pragma once

#include <string>
#include <vector>

namespace game
{
enum class Difficulty
{
    EASY,     // 简单
    NORMAL,   // 普通
    HARD,     // 困难
    EXTREME   // 极难
};

struct DifficultySettings
{
    Difficulty difficulty;
    std::string name;
    std::string description;
    
    float ballSpeedMultiplier;      // 球速倍数
    float paddleSpeedMultiplier;    // 挡板速度倍数
    float paddleWidthMultiplier;    // 挡板宽度倍数
    float brickHealthMultiplier;    // 砖块生命倍数
    float enemySpawnRateMultiplier; // 敌人生成速率倍数
    
    int initialLives;               // 初始生命数
    float failureForgivenessTime;   // 失败宽恕时间（秒）
    
    float scoreMultiplier;          // 分数倍数
    float powerUpDropRateMultiplier; // 道具下降倍数
};

class DifficultyManager
{
public:
    static constexpr int DIFFICULTY_COUNT = 4;
    
    DifficultyManager();
    
    void setDifficulty(Difficulty difficulty);
    Difficulty getCurrentDifficulty() const noexcept { return currentDifficulty_; }
    const DifficultySettings& getCurrentSettings() const noexcept { return settings_; }
    
    const DifficultySettings& getSettings(Difficulty difficulty) const;
    
    std::vector<DifficultySettings> getAvailableDifficulties() const;
    
    void autoAdjustDifficulty(bool playerSucceeding, int consecutiveWins, int consecutiveLosses);
    
private:
    Difficulty currentDifficulty_;
    DifficultySettings settings_;
    
    void initializeDefaultSettings();
};

} // namespace game
