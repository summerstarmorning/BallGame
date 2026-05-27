#include "GameEnhancementManager.hpp"

namespace game
{

GameEnhancementManager::GameEnhancementManager() : initialized_(false)
{
}

GameEnhancementManager::~GameEnhancementManager()
{
    if (initialized_)
        shutdown();
}

void GameEnhancementManager::initialize()
{
    if (initialized_) return;
    
    // 初始化所有子系统
    soundManager_.initialize();
    achievementSystem_.initialize();
    leaderboardSystem_.loadLocalLeaderboard("leaderboard.json");
    tutorialSystem_.initialize();
    
    initialized_ = true;
}

void GameEnhancementManager::shutdown()
{
    if (!initialized_) return;
    
    // 保存排行榜
    leaderboardSystem_.saveLocalLeaderboard("leaderboard.json");
    
    // 关闭音效系统
    soundManager_.shutdown();
    
    initialized_ = false;
}

void GameEnhancementManager::update(float deltaSeconds)
{
    if (!initialized_) return;
    
    // 更新各个系统
    comboSystem_.update(deltaSeconds);
    soundManager_.update();
    statisticsTracker_.update(deltaSeconds);
    tutorialSystem_.update(deltaSeconds);
}

void GameEnhancementManager::onGameStart()
{
    // 游戏开始事件
    statisticsTracker_.reset();
    comboSystem_.resetCombo();
    soundManager_.playSound(SoundType::MENU_CONFIRM);
}

void GameEnhancementManager::onGameEnd(int finalScore, float survivalTime)
{
    // 游戏结束事件
    statisticsTracker_.getMutableStatistics().survivalTimeSeconds = survivalTime;
    statisticsTracker_.getMutableStatistics().scoreThisRun = finalScore;
    achievementSystem_.onGameOver(finalScore, survivalTime);
    
    // 检查是否是新的个人最佳
    if (leaderboardSystem_.isNewPersonalBest(finalScore))
    {
        soundManager_.playSound(SoundType::ACHIEVEMENT_UNLOCK);
    }
}

void GameEnhancementManager::onBrickDestroyed()
{
    // 砖块被击碎
    statisticsTracker_.recordBrickDestroyed();
    achievementSystem_.onBrickDestroyed(statisticsTracker_.getStatistics().lifetimeBricksDestroyed);
    soundManager_.playSound(SoundType::BRICK_HIT);
    comboSystem_.addHit();
}

void GameEnhancementManager::onBallHit()
{
    // 球被击中
    statisticsTracker_.recordHit();
    soundManager_.playSound(SoundType::BALL_PADDLE_HIT);
}

void GameEnhancementManager::onComboMilestone(int comboCount)
{
    // 连击里程碑
    achievementSystem_.onComboChanged(comboCount);
    statisticsTracker_.recordCombo(comboCount);
    
    // 播放相应的音效
    if (comboCount % 10 == 0)
    {
        soundManager_.playSoundEx(SoundType::COMBO_MILESTONE, 1.0f, 1.0f + (comboCount / 100.0f));
    }
}

void GameEnhancementManager::onAchievementUnlocked(AchievementId achievement)
{
    // 成就解锁
    soundManager_.playSound(SoundType::ACHIEVEMENT_UNLOCK);
}

void GameEnhancementManager::onLevelComplete(int levelNumber, int totalLevels, bool perfect)
{
    // 关卡完成
    statisticsTracker_.recordLevelComplete(perfect);
    achievementSystem_.onLevelComplete(levelNumber, totalLevels, perfect ? 1 : 0);
    soundManager_.playSound(SoundType::LEVEL_COMPLETE);
}

} // namespace game
