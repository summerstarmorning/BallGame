#include "AchievementSystem.hpp"
#include <chrono>
#include <fstream>
#include "nlohmann_json.hpp"

using json = nlohmann::json;

namespace game
{

AchievementSystem::AchievementSystem()
{
    achievements_.resize(static_cast<int>(AchievementId::ACHIEVEMENT_COUNT));
    initializeAchievements();
}

void AchievementSystem::initializeAchievements()
{
    // 初始化所有成就定义
    std::vector<Achievement> defs;
    
    Achievement a;
    
    // FIRST_BRICK
    a.id = AchievementId::FIRST_BRICK;
    a.nameZh = u8"初次尝试";
    a.nameEn = "First Strike";
    a.descriptionZh = u8"击碎第一块砖";
    a.descriptionEn = "Destroy your first brick";
    a.iconEmoji = u8"🧱";
    a.rewardSoftCurrency = 10;
    a.unlocked = false;
    a.unlockedAtTimestamp = 0;
    a.unlockProgress = 0.0f;
    achievements_[0] = a;
    
    // BRICKS_100
    a.id = AchievementId::BRICKS_100;
    a.nameZh = u8"砖块收割机";
    a.nameEn = "Brick Harvester";
    a.descriptionZh = u8"累计击碎100块砖";
    a.descriptionEn = "Destroy 100 bricks";
    a.iconEmoji = u8"🔨";
    a.rewardSoftCurrency = 50;
    achievements_[1] = a;
    
    // BRICKS_1000
    a.id = AchievementId::BRICKS_1000;
    a.nameZh = u8"砖块大师";
    a.nameEn = "Brick Master";
    a.descriptionZh = u8"累计击碎1000块砖";
    a.descriptionEn = "Destroy 1000 bricks";
    a.iconEmoji = u8"💎";
    a.rewardSoftCurrency = 200;
    achievements_[2] = a;
    
    // SCORE_1000
    a.id = AchievementId::SCORE_1000;
    a.nameZh = u8"初入境界";
    a.nameEn = "Score Novice";
    a.descriptionZh = u8"获得1000分";
    a.descriptionEn = "Reach 1000 points";
    a.iconEmoji = u8"⭐";
    a.rewardSoftCurrency = 25;
    achievements_[3] = a;
    
    // SCORE_10000
    a.id = AchievementId::SCORE_10000;
    a.nameZh = u8"分数高手";
    a.nameEn = "Score Expert";
    a.descriptionZh = u8"获得10000分";
    a.descriptionEn = "Reach 10000 points";
    a.iconEmoji = u8"⭐⭐";
    a.rewardSoftCurrency = 100;
    achievements_[4] = a;
    
    // SCORE_100000
    a.id = AchievementId::SCORE_100000;
    a.nameZh = u8"分数之王";
    a.nameEn = "Score Legend";
    a.descriptionZh = u8"获得100000分";
    a.descriptionEn = "Reach 100000 points";
    a.iconEmoji = u8"👑";
    a.rewardSoftCurrency = 500;
    achievements_[5] = a;
    
    // COMBO_10
    a.id = AchievementId::COMBO_10;
    a.nameZh = u8"连击小能手";
    a.nameEn = "Combo Starter";
    a.descriptionZh = u8"达成10连击";
    a.descriptionEn = "Achieve 10 combo";
    a.iconEmoji = u8"🔥";
    a.rewardSoftCurrency = 30;
    achievements_[6] = a;
    
    // COMBO_25
    a.id = AchievementId::COMBO_25;
    a.nameZh = u8"连击高手";
    a.nameEn = "Combo Expert";
    a.descriptionZh = u8"达成25连击";
    a.descriptionEn = "Achieve 25 combo";
    a.iconEmoji = u8"🔥🔥";
    a.rewardSoftCurrency = 75;
    achievements_[7] = a;
    
    // COMBO_50
    a.id = AchievementId::COMBO_50;
    a.nameZh = u8"连击大师";
    a.nameEn = "Combo Master";
    a.descriptionZh = u8"达成50连击";
    a.descriptionEn = "Achieve 50 combo";
    a.iconEmoji = u8"🔥🔥🔥";
    a.rewardSoftCurrency = 150;
    achievements_[8] = a;
    
    // COMBO_100
    a.id = AchievementId::COMBO_100;
    a.nameZh = u8"连击之神";
    a.nameEn = "Combo God";
    a.descriptionZh = u8"达成100连击";
    a.descriptionEn = "Achieve 100 combo";
    a.iconEmoji = u8"💫";
    a.rewardSoftCurrency = 300;
    achievements_[9] = a;
    
    // SURVIVE_5_LEVELS
    a.id = AchievementId::SURVIVE_5_LEVELS;
    a.nameZh = u8"旅者";
    a.nameEn = "Traveler";
    a.descriptionZh = u8"完成5个关卡";
    a.descriptionEn = "Complete 5 levels";
    a.iconEmoji = u8"🌍";
    a.rewardSoftCurrency = 50;
    achievements_[10] = a;
    
    // SURVIVE_10_LEVELS
    a.id = AchievementId::SURVIVE_10_LEVELS;
    a.nameZh = u8"冒险家";
    a.nameEn = "Adventurer";
    a.descriptionZh = u8"完成10个关卡";
    a.descriptionEn = "Complete 10 levels";
    a.iconEmoji = u8"🗺️";
    a.rewardSoftCurrency = 150;
    achievements_[11] = a;
    
    // SURVIVE_ALL_LEVELS
    a.id = AchievementId::SURVIVE_ALL_LEVELS;
    a.nameZh = u8"传奇冒险家";
    a.nameEn = "Legend";
    a.descriptionZh = u8"完成所有关卡";
    a.descriptionEn = "Complete all levels";
    a.iconEmoji = u8"🏆";
    a.rewardSoftCurrency = 500;
    achievements_[12] = a;
    
    // COLLECT_10_POWERUPS
    a.id = AchievementId::COLLECT_10_POWERUPS;
    a.nameZh = u8"道具收集者";
    a.nameEn = "Collector";
    a.descriptionZh = u8"收集10个道具";
    a.descriptionEn = "Collect 10 power-ups";
    a.iconEmoji = u8"🎁";
    a.rewardSoftCurrency = 40;
    achievements_[13] = a;
    
    // COLLECT_50_POWERUPS
    a.id = AchievementId::COLLECT_50_POWERUPS;
    a.nameZh = u8"道具大王";
    a.nameEn = "Power Collector";
    a.descriptionZh = u8"收集50个道具";
    a.descriptionEn = "Collect 50 power-ups";
    a.iconEmoji = u8"🎁🎁";
    a.rewardSoftCurrency = 120;
    achievements_[14] = a;
    
    // COLLECT_ALL_POWERUP_TYPES
    a.id = AchievementId::COLLECT_ALL_POWERUP_TYPES;
    a.nameZh = u8"道具猎人";
    a.nameEn = "Power Hunter";
    a.descriptionZh = u8"收集所有类型的道具";
    a.descriptionEn = "Collect all power-up types";
    a.iconEmoji = u8"🎯";
    a.rewardSoftCurrency = 200;
    achievements_[15] = a;
    
    // HARD_MODE_COMPLETED
    a.id = AchievementId::HARD_MODE_COMPLETED;
    a.nameZh = u8"硬汉";
    a.nameEn = "Hardcore Gamer";
    a.descriptionZh = u8"困难模式通关";
    a.descriptionEn = "Complete Hard mode";
    a.iconEmoji = u8"💪";
    a.rewardSoftCurrency = 200;
    achievements_[16] = a;
    
    // EXTREME_MODE_COMPLETED
    a.id = AchievementId::EXTREME_MODE_COMPLETED;
    a.nameZh = u8"极难挑战者";
    a.nameEn = "Extreme Master";
    a.descriptionZh = u8"极难模式通关";
    a.descriptionEn = "Complete Extreme mode";
    a.iconEmoji = u8"⚡";
    a.rewardSoftCurrency = 500;
    achievements_[17] = a;
    
    // PERFECT_LEVEL
    a.id = AchievementId::PERFECT_LEVEL;
    a.nameZh = u8"完美通过";
    a.nameEn = "Perfect Pass";
    a.descriptionZh = u8"一条命完成关卡（不失一条命）";
    a.descriptionEn = "Complete level without losing a life";
    a.iconEmoji = u8"✨";
    a.rewardSoftCurrency = 100;
    achievements_[18] = a;
    
    // NO_DEATH_SPEED_RUN
    a.id = AchievementId::NO_DEATH_SPEED_RUN;
    a.nameZh = u8"闪电战士";
    a.nameEn = "Speed Demon";
    a.descriptionZh = u8"快速无死亡通关";
    a.descriptionEn = "Quick run without deaths";
    a.iconEmoji = u8"⚡⚡";
    a.rewardSoftCurrency = 250;
    achievements_[19] = a;
    
    // GODLIKE
    a.id = AchievementId::GODLIKE;
    a.nameZh = u8"无敌之神";
    a.nameEn = "Godlike";
    a.descriptionZh = u8"连续3关不失一条命";
    a.descriptionEn = "Complete 3 levels without losing a life";
    a.iconEmoji = u8"🌟";
    a.rewardSoftCurrency = 400;
    achievements_[20] = a;
    
    // LUCKY
    a.id = AchievementId::LUCKY;
    a.nameZh = u8"幸运儿";
    a.nameEn = "Lucky";
    a.descriptionZh = u8"一次获得3个相同道具";
    a.descriptionEn = "Get 3 same power-ups at once";
    a.iconEmoji = u8"🍀";
    a.rewardSoftCurrency = 150;
    achievements_[21] = a;
    
    // SHARPSHOOTER
    a.id = AchievementId::SHARPSHOOTER;
    a.nameZh = u8"神射手";
    a.nameEn = "Sharpshooter";
    a.descriptionZh = u8"击中率超过95%";
    a.descriptionEn = "Accuracy over 95%";
    a.iconEmoji = u8"🎯";
    a.rewardSoftCurrency = 200;
    achievements_[22] = a;
    
    // 构建查询表
    for (size_t i = 0; i < achievements_.size(); ++i)
    {
        achievementMap_[static_cast<std::uint8_t>(achievements_[i].id)] = &achievements_[i];
    }
}

void AchievementSystem::unlockAchievement(AchievementId achievementId)
{
    Achievement& achievement = achievements_[static_cast<int>(achievementId)];
    if (!achievement.unlocked)
    {
        achievement.unlocked = true;
        achievement.unlockedAtTimestamp = 
            std::chrono::system_clock::now().time_since_epoch().count();
    }
}

void AchievementSystem::setProgress(AchievementId achievementId, float progress)
{
    if (static_cast<int>(achievementId) < achievements_.size())
    {
        achievements_[static_cast<int>(achievementId)].unlockProgress = 
            std::max(0.0f, std::min(1.0f, progress));
    }
}

const Achievement& AchievementSystem::getAchievement(AchievementId achievementId) const
{
    return achievements_[static_cast<int>(achievementId)];
}

int AchievementSystem::getUnlockedCount() const
{
    int count = 0;
    for (const auto& ach : achievements_)
    {
        if (ach.unlocked) count++;
    }
    return count;
}

int AchievementSystem::getTotalReward() const
{
    int total = 0;
    for (const auto& ach : achievements_)
    {
        if (ach.unlocked)
            total += ach.rewardSoftCurrency;
    }
    return total;
}

bool AchievementSystem::isUnlocked(AchievementId achievementId) const
{
    return achievements_[static_cast<int>(achievementId)].unlocked;
}

void AchievementSystem::save(const std::string& profilePath)
{
    // TODO: 实现成就存储
}

void AchievementSystem::load(const std::string& profilePath)
{
    // TODO: 实现成就加载
}

void AchievementSystem::onBrickDestroyed(int totalBricksDestroyed)
{
    if (totalBricksDestroyed == 1)
        unlockAchievement(AchievementId::FIRST_BRICK);
    
    setProgress(AchievementId::BRICKS_100, 
                std::min(1.0f, totalBricksDestroyed / 100.0f));
    if (totalBricksDestroyed >= 100)
        unlockAchievement(AchievementId::BRICKS_100);
    
    setProgress(AchievementId::BRICKS_1000, 
                std::min(1.0f, totalBricksDestroyed / 1000.0f));
    if (totalBricksDestroyed >= 1000)
        unlockAchievement(AchievementId::BRICKS_1000);
}

void AchievementSystem::onScoreChanged(int currentScore)
{
    setProgress(AchievementId::SCORE_1000, 
                std::min(1.0f, currentScore / 1000.0f));
    if (currentScore >= 1000)
        unlockAchievement(AchievementId::SCORE_1000);
    
    setProgress(AchievementId::SCORE_10000, 
                std::min(1.0f, currentScore / 10000.0f));
    if (currentScore >= 10000)
        unlockAchievement(AchievementId::SCORE_10000);
    
    setProgress(AchievementId::SCORE_100000, 
                std::min(1.0f, currentScore / 100000.0f));
    if (currentScore >= 100000)
        unlockAchievement(AchievementId::SCORE_100000);
}

void AchievementSystem::onComboChanged(int currentCombo)
{
    setProgress(AchievementId::COMBO_10, std::min(1.0f, currentCombo / 10.0f));
    if (currentCombo >= 10)
        unlockAchievement(AchievementId::COMBO_10);
    
    setProgress(AchievementId::COMBO_25, std::min(1.0f, currentCombo / 25.0f));
    if (currentCombo >= 25)
        unlockAchievement(AchievementId::COMBO_25);
    
    setProgress(AchievementId::COMBO_50, std::min(1.0f, currentCombo / 50.0f));
    if (currentCombo >= 50)
        unlockAchievement(AchievementId::COMBO_50);
    
    setProgress(AchievementId::COMBO_100, std::min(1.0f, currentCombo / 100.0f));
    if (currentCombo >= 100)
        unlockAchievement(AchievementId::COMBO_100);
}

void AchievementSystem::onLevelComplete(int levelNumber, int totalLevels, int livesRemaining)
{
    setProgress(AchievementId::SURVIVE_5_LEVELS, std::min(1.0f, levelNumber / 5.0f));
    if (levelNumber >= 5)
        unlockAchievement(AchievementId::SURVIVE_5_LEVELS);
    
    setProgress(AchievementId::SURVIVE_10_LEVELS, std::min(1.0f, levelNumber / 10.0f));
    if (levelNumber >= 10)
        unlockAchievement(AchievementId::SURVIVE_10_LEVELS);
    
    if (levelNumber >= totalLevels)
        unlockAchievement(AchievementId::SURVIVE_ALL_LEVELS);
    
    if (livesRemaining > 0)
        unlockAchievement(AchievementId::PERFECT_LEVEL);
}

void AchievementSystem::onPowerUpCollected(int totalCollected)
{
    setProgress(AchievementId::COLLECT_10_POWERUPS, std::min(1.0f, totalCollected / 10.0f));
    if (totalCollected >= 10)
        unlockAchievement(AchievementId::COLLECT_10_POWERUPS);
    
    setProgress(AchievementId::COLLECT_50_POWERUPS, std::min(1.0f, totalCollected / 50.0f));
    if (totalCollected >= 50)
        unlockAchievement(AchievementId::COLLECT_50_POWERUPS);
}

void AchievementSystem::onGameOver(int finalScore, float survivedTime)
{
    onScoreChanged(finalScore);
}

} // namespace game
