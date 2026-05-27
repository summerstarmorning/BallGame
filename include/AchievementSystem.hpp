#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace game
{

enum class AchievementId : std::uint8_t
{
    // 基础成就
    FIRST_BRICK = 0,           // 击碎第一块砖
    BRICKS_100,                // 击碎100块砖
    BRICKS_1000,               // 击碎1000块砖
    
    // 分数成就
    SCORE_1000,                // 获得1000分
    SCORE_10000,               // 获得10000分
    SCORE_100000,              // 获得100000分
    
    // 连击成就
    COMBO_10,                  // 10连击
    COMBO_25,                  // 25连击
    COMBO_50,                  // 50连击
    COMBO_100,                 // 100连击
    
    // 生存成就
    SURVIVE_5_LEVELS,          // 完成5关
    SURVIVE_10_LEVELS,         // 完成10关
    SURVIVE_ALL_LEVELS,        // 完成所有关卡
    
    // 道具成就
    COLLECT_10_POWERUPS,       // 收集10个道具
    COLLECT_50_POWERUPS,       // 收集50个道具
    COLLECT_ALL_POWERUP_TYPES, // 收集所有类型道具
    
    // 挑战成就
    HARD_MODE_COMPLETED,       // 困难模式通关
    EXTREME_MODE_COMPLETED,    // 极难模式通关
    PERFECT_LEVEL,             // 完美通关（不失一条命）
    NO_DEATH_SPEED_RUN,        // 不死速通（快速通关）
    
    // 特殊成就
    GODLIKE,                   // 无敌（连续不失命完成3关）
    LUCKY,                     // 幸运儿（一次获得3个相同道具）
    SHARPSHOOTER,              // 枪法精准（击中率>95%）
    
    ACHIEVEMENT_COUNT
};

struct Achievement
{
    AchievementId id;
    std::string nameZh;
    std::string nameEn;
    std::string descriptionZh;
    std::string descriptionEn;
    std::string iconEmoji;
    int rewardSoftCurrency;
    bool unlocked;
    uint64_t unlockedAtTimestamp;
    float unlockProgress;  // 0.0 ~ 1.0，用于显示进度条
};

class AchievementSystem
{
public:
    AchievementSystem();
    
    void initialize();
    void unlockAchievement(AchievementId achievementId);
    void setProgress(AchievementId achievementId, float progress);
    
    const Achievement& getAchievement(AchievementId achievementId) const;
    const std::vector<Achievement>& getAllAchievements() const noexcept { return achievements_; }
    
    int getUnlockedCount() const;
    int getTotalReward() const;
    bool isUnlocked(AchievementId achievementId) const;
    
    void save(const std::string& profilePath);
    void load(const std::string& profilePath);
    
    // 便捷方法用于游戏中的事件
    void onBrickDestroyed(int totalBricksDestroyed);
    void onScoreChanged(int currentScore);
    void onComboChanged(int currentCombo);
    void onLevelComplete(int levelNumber, int totalLevels, int livesRemaining);
    void onPowerUpCollected(int totalCollected);
    void onGameOver(int finalScore, float survivedTime);
    
private:
    std::vector<Achievement> achievements_;
    std::unordered_map<std::uint8_t, Achievement*> achievementMap_;
    
    void initializeAchievements();
};

} // namespace game
