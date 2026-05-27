#include "Config/DifficultyConfig.hpp"

namespace game
{

DifficultyManager::DifficultyManager() : currentDifficulty_(Difficulty::NORMAL)
{
    initializeDefaultSettings();
    setDifficulty(Difficulty::NORMAL);
}

void DifficultyManager::initializeDefaultSettings()
{
    // 这个函数在setDifficulty中实现每个难度的设置
}

void DifficultyManager::setDifficulty(Difficulty difficulty)
{
    currentDifficulty_ = difficulty;
    
    switch (difficulty)
    {
        case Difficulty::EASY:
            settings_.difficulty = Difficulty::EASY;
            settings_.name = "简单";
            settings_.description = u8"完美的新手入门模式";
            settings_.ballSpeedMultiplier = 0.75f;
            settings_.paddleSpeedMultiplier = 1.3f;
            settings_.paddleWidthMultiplier = 1.3f;
            settings_.brickHealthMultiplier = 0.8f;
            settings_.enemySpawnRateMultiplier = 0.6f;
            settings_.initialLives = 5;
            settings_.failureForgivenessTime = 2.0f;
            settings_.scoreMultiplier = 0.5f;
            settings_.powerUpDropRateMultiplier = 1.5f;
            break;
            
        case Difficulty::NORMAL:
            settings_.difficulty = Difficulty::NORMAL;
            settings_.name = u8"普通";
            settings_.description = u8"标准游戏体验";
            settings_.ballSpeedMultiplier = 1.0f;
            settings_.paddleSpeedMultiplier = 1.0f;
            settings_.paddleWidthMultiplier = 1.0f;
            settings_.brickHealthMultiplier = 1.0f;
            settings_.enemySpawnRateMultiplier = 1.0f;
            settings_.initialLives = 3;
            settings_.failureForgivenessTime = 1.0f;
            settings_.scoreMultiplier = 1.0f;
            settings_.powerUpDropRateMultiplier = 1.0f;
            break;
            
        case Difficulty::HARD:
            settings_.difficulty = Difficulty::HARD;
            settings_.name = u8"困难";
            settings_.description = u8"给那些富有挑战精神的玩家";
            settings_.ballSpeedMultiplier = 1.35f;
            settings_.paddleSpeedMultiplier = 0.9f;
            settings_.paddleWidthMultiplier = 0.85f;
            settings_.brickHealthMultiplier = 1.2f;
            settings_.enemySpawnRateMultiplier = 1.3f;
            settings_.initialLives = 2;
            settings_.failureForgivenessTime = 0.5f;
            settings_.scoreMultiplier = 1.5f;
            settings_.powerUpDropRateMultiplier = 0.8f;
            break;
            
        case Difficulty::EXTREME:
            settings_.difficulty = Difficulty::EXTREME;
            settings_.name = u8"极难";
            settings_.description = u8"仅限高手玩家";
            settings_.ballSpeedMultiplier = 1.7f;
            settings_.paddleSpeedMultiplier = 0.8f;
            settings_.paddleWidthMultiplier = 0.7f;
            settings_.brickHealthMultiplier = 1.5f;
            settings_.enemySpawnRateMultiplier = 1.6f;
            settings_.initialLives = 1;
            settings_.failureForgivenessTime = 0.2f;
            settings_.scoreMultiplier = 2.5f;
            settings_.powerUpDropRateMultiplier = 0.6f;
            break;
    }
}

const DifficultySettings& DifficultyManager::getSettings(Difficulty difficulty) const
{
    static DifficultyManager tempManager;
    return tempManager.getAvailableDifficulties()[static_cast<int>(difficulty)];
}

std::vector<DifficultySettings> DifficultyManager::getAvailableDifficulties() const
{
    std::vector<DifficultySettings> difficulties;
    
    for (int i = 0; i < DIFFICULTY_COUNT; ++i)
    {
        DifficultyManager tempMgr;
        tempMgr.setDifficulty(static_cast<Difficulty>(i));
        difficulties.push_back(tempMgr.getCurrentSettings());
    }
    
    return difficulties;
}

void DifficultyManager::autoAdjustDifficulty(bool playerSucceeding, int consecutiveWins, int consecutiveLosses)
{
    // 动态难度调整逻辑
    // 连续成功3次：提高难度
    if (playerSucceeding && consecutiveWins >= 3 && currentDifficulty_ < Difficulty::EXTREME)
    {
        setDifficulty(static_cast<Difficulty>(static_cast<int>(currentDifficulty_) + 1));
        return;
    }
    
    // 连续失败2次：降低难度
    if (!playerSucceeding && consecutiveLosses >= 2 && currentDifficulty_ > Difficulty::EASY)
    {
        setDifficulty(static_cast<Difficulty>(static_cast<int>(currentDifficulty_) - 1));
    }
}

} // namespace game
