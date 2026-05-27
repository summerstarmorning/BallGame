#pragma once

#include "Config/DifficultyConfig.hpp"
#include "ComboSystem.hpp"
#include "AchievementSystem.hpp"
#include "StatisticsTracker.hpp"
#include "SoundManager.hpp"
#include "LeaderboardSystem.hpp"
#include "TutorialSystem.hpp"
#include "BossEnemy.hpp"

namespace game
{

class GameEnhancementManager
{
public:
    GameEnhancementManager();
    ~GameEnhancementManager();
    
    // 初始化所有子系统
    void initialize();
    void shutdown();
    
    // 获取各个系统的引用
    DifficultyManager& getDifficultyManager() noexcept { return difficultyManager_; }
    ComboSystem& getComboSystem() noexcept { return comboSystem_; }
    AchievementSystem& getAchievementSystem() noexcept { return achievementSystem_; }
    StatisticsTracker& getStatisticsTracker() noexcept { return statisticsTracker_; }
    SoundManager& getSoundManager() noexcept { return soundManager_; }
    LeaderboardSystem& getLeaderboardSystem() noexcept { return leaderboardSystem_; }
    TutorialSystem& getTutorialSystem() noexcept { return tutorialSystem_; }
    
    const DifficultyManager& getDifficultyManager() const noexcept { return difficultyManager_; }
    const ComboSystem& getComboSystem() const noexcept { return comboSystem_; }
    const AchievementSystem& getAchievementSystem() const noexcept { return achievementSystem_; }
    const StatisticsTracker& getStatisticsTracker() const noexcept { return statisticsTracker_; }
    const SoundManager& getSoundManager() const noexcept { return soundManager_; }
    const LeaderboardSystem& getLeaderboardSystem() const noexcept { return leaderboardSystem_; }
    const TutorialSystem& getTutorialSystem() const noexcept { return tutorialSystem_; }
    
    // 更新所有系统
    void update(float deltaSeconds);
    
    // 事件处理
    void onGameStart();
    void onGameEnd(int finalScore, float survivalTime);
    void onBrickDestroyed();
    void onBallHit();
    void onComboMilestone(int comboCount);
    void onAchievementUnlocked(AchievementId achievement);
    void onLevelComplete(int levelNumber, int totalLevels, bool perfect);
    
private:
    DifficultyManager difficultyManager_;
    ComboSystem comboSystem_;
    AchievementSystem achievementSystem_;
    StatisticsTracker statisticsTracker_;
    SoundManager soundManager_;
    LeaderboardSystem leaderboardSystem_;
    TutorialSystem tutorialSystem_;
    
    bool initialized_;
};

} // namespace game
