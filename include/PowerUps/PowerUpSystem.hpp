#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Config/PowerUpConfig.hpp"
#include "GameTypes.hpp"
#include "PowerUps/PowerUp.hpp"

namespace game
{
class GameWorld;
class PowerUpEffect;
class PowerUpFactory;

class PowerUpSystem
{
public:
    PowerUpSystem();

    void setConfigSet(const PowerUpConfigSet& configSet);
    bool trySpawnFromBrickBreak(PowerUpType type, const Vec2& position, float random01);
    void update(float deltaSeconds, GameWorld& world, float offScreenY);
    void render() const;

    void onLevelTransition(GameWorld& world, bool keepEffects);
    void clear(GameWorld& world);

    const std::vector<PowerUp>& droppedPowerUps() const noexcept { return droppedPowerUps_; }
    const std::vector<std::unique_ptr<PowerUpEffect>>& activeEffects() const noexcept { return activeEffects_; }

private:
    void activate(PowerUpType type, GameWorld& world);
    void cleanupDrops(float offScreenY);
    void cleanupFinishedEffects(GameWorld& world);
    void syncWorldEffects(GameWorld& world) noexcept;

    std::unordered_map<PowerUpType, PowerUpConfig> configByType_ {};
    ParticleConfig particleConfig_ {};
    std::vector<PowerUp> droppedPowerUps_ {};
    std::vector<std::unique_ptr<PowerUpEffect>> activeEffects_ {};
};
} // namespace game
