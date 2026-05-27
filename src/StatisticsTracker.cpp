#include "StatisticsTracker.hpp"

namespace game
{

StatisticsTracker::StatisticsTracker() : timeSinceLastUpdate_(0.0f)
{
    reset();
}

void StatisticsTracker::reset()
{
    stats_.bricksDestroyedThisRun = 0;
    stats_.powerUpsCollectedThisRun = 0;
    stats_.scoreThisRun = 0;
    stats_.maxComboThisRun = 0;
    stats_.survivalTimeSeconds = 0.0f;
    stats_.accuracyHits = 0;
    stats_.accurateMisses = 0;
}

void StatisticsTracker::recordHit()
{
    stats_.accuracyHits++;
}

void StatisticsTracker::recordMiss()
{
    stats_.accurateMisses++;
}

void StatisticsTracker::recordBrickDestroyed()
{
    stats_.bricksDestroyedThisRun++;
    stats_.lifetimeBricksDestroyed++;
}

void StatisticsTracker::recordPowerUpCollected()
{
    stats_.powerUpsCollectedThisRun++;
    stats_.lifetimePowerUpsCollected++;
}

void StatisticsTracker::recordScore(int score)
{
    stats_.scoreThisRun = score;
    stats_.lifetimeTotalScore += score;
    if (score > stats_.lifetimeBestScore)
        stats_.lifetimeBestScore = score;
}

void StatisticsTracker::recordCombo(int combo)
{
    if (combo > stats_.maxComboThisRun)
        stats_.maxComboThisRun = combo;
    if (combo > stats_.lifetimeMaxCombo)
        stats_.lifetimeMaxCombo = combo;
}

void StatisticsTracker::recordLevelComplete(bool perfect)
{
    stats_.lifetimeLevelsCompleted++;
    if (perfect)
        stats_.lifetimePerfectLevels++;
}

void StatisticsTracker::update(float deltaSeconds)
{
    stats_.survivalTimeSeconds += deltaSeconds;
    stats_.lifetimeTotalPlayTime += deltaSeconds;
}

void StatisticsTracker::finalizeSeason()
{
    // 结束一个赛季时的处理
    reset();
}

} // namespace game
