#pragma once

#include "Config/PowerUpConfig.hpp"

namespace game
{
class GameWorld;

class PowerUpEffect
{
public:
    explicit PowerUpEffect(PowerUpConfig config) : config_(config) {}
    virtual ~PowerUpEffect() = default;

    virtual void apply(GameWorld& world) = 0;
    virtual void update(GameWorld& world, float deltaSeconds);
    virtual void expire(GameWorld& world) = 0;
    virtual bool shouldReapplyOnLevelTransition() const noexcept { return true; }
    virtual void onLevelStarted(GameWorld& world);

    bool isFinished() const noexcept;
    bool isPermanent() const noexcept { return config_.permanent; }
    PowerUpType type() const noexcept { return config_.type; }
    float durationSeconds() const noexcept { return config_.durationSeconds; }
    float magnitude() const noexcept { return config_.magnitude; }
    float totalDurationSeconds() const noexcept;
    float remainingSeconds() const noexcept;
    void extendDuration(float extraSeconds) noexcept;
    void forceExpire(GameWorld& world);

protected:
    const PowerUpConfig& config() const noexcept { return config_; }
    void finish(GameWorld& world);

private:
    PowerUpConfig config_ {};
    float elapsedSeconds_ {0.0F};
    float bonusDurationSeconds_ {0.0F};
    bool finished_ {false};
    bool expired_ {false};
};
} // namespace game
