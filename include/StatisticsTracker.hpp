#pragma once

#include <string>

namespace game
{

struct GameStatistics
{
    // 单局统计
    int bricksDestroyedThisRun = 0;
    int powerUpsCollectedThisRun = 0;
    int scoreThisRun = 0;
    int maxComboThisRun = 0;
    float survivalTimeSeconds = 0.0f;
    int accuracyHits = 0;
    int accurateMisses = 0;
    
    // 生涯统计
    int lifetimeBricksDestroyed = 0;
    int lifetimePowerUpsCollected = 0;
    int lifetimeTotalScore = 0;
    int lifetimeBestScore = 0;
    int lifetimeMaxCombo = 0;
    int lifetimeLevelsCompleted = 0;
    int lifetimePerfectLevels = 0;
    float lifetimeTotalPlayTime = 0.0f;
    
    float getAccuracy() const
    {
        int total = accuracyHits + accurateMisses;
        if (total == 0) return 0.0f;
        return (accuracyHits * 100.0f) / total;
    }
    
    std::string getAccuracyString() const
    {
        return std::to_string(static_cast<int>(getAccuracy())) + "%";
    }
};

class StatisticsTracker
{
public:
    StatisticsTracker();
    
    void reset();
    void recordHit();
    void recordMiss();
    void recordBrickDestroyed();
    void recordPowerUpCollected();
    void recordScore(int score);
    void recordCombo(int combo);
    void recordLevelComplete(bool perfect);
    void update(float deltaSeconds);
    
    const GameStatistics& getStatistics() const noexcept { return stats_; }
    GameStatistics& getMutableStatistics() noexcept { return stats_; }
    
    void finalizeSeason();
    
private:
    GameStatistics stats_;
    float timeSinceLastUpdate_;
};

} // namespace game
