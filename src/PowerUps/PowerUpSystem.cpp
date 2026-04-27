#include "PowerUps/PowerUpSystem.hpp"

#include <algorithm>

#include "Effects/PowerUpEffect.hpp"
#include "GameWorld.hpp"
#include "Paddle.hpp"
#include "Particles/ParticleSystem.hpp"
#include "PowerUps/PowerUpFactory.hpp"

namespace game
{
namespace
{
constexpr float kPowerUpFallSpeed = 260.0F;
}

PowerUpSystem::PowerUpSystem() = default;

void PowerUpSystem::setConfigSet(const PowerUpConfigSet& configSet)
{
    configByType_ = configSet.powerUps;
    particleConfig_ = configSet.particles;
}

bool PowerUpSystem::trySpawnFromBrickBreak(PowerUpType type, const Vec2& position, float random01)
{
    const auto configIt = configByType_.find(type);
    if (configIt == configByType_.end())
    {
        return false;
    }

    if (random01 > configIt->second.dropChance)
    {
        return false;
    }

    droppedPowerUps_.emplace_back(type, position, Vec2 {0.0F, kPowerUpFallSpeed});
    return true;
}

void PowerUpSystem::update(float deltaSeconds, GameWorld& world, float offScreenY)
{
    for (PowerUp& powerUp : droppedPowerUps_)
    {
        if (powerUp.isCollected())
        {
            continue;
        }

        powerUp.update(deltaSeconds);

        if (world.particleSystem != nullptr && particleConfig_.powerUpTrailCount > 0U)
        {
            world.particleSystem->spawnPowerUpTrail(powerUp.position(), particleConfig_.powerUpTrailCount);
        }

        if (world.paddle != nullptr && powerUp.checkPickup(*world.paddle))
        {
            powerUp.markCollected();
            if (world.playerProfile != nullptr)
            {
                ++world.playerProfile->totalPowerUpsCollected;
            }
            activate(powerUp.type(), world);
        }
    }

    cleanupDrops(offScreenY);
    syncWorldEffects(world);

    for (std::unique_ptr<PowerUpEffect>& effect : activeEffects_)
    {
        effect->update(world, deltaSeconds);
    }

    cleanupFinishedEffects(world);
    syncWorldEffects(world);
}

void PowerUpSystem::render() const
{
    for (const PowerUp& powerUp : droppedPowerUps_)
    {
        if (!powerUp.isCollected())
        {
            powerUp.render();
        }
    }
}

void PowerUpSystem::onLevelTransition(GameWorld& world, bool keepEffects)
{
    droppedPowerUps_.clear();

    if (!keepEffects)
    {
        clear(world);
        return;
    }

    syncWorldEffects(world);
    for (std::unique_ptr<PowerUpEffect>& effect : activeEffects_)
    {
        effect->onLevelStarted(world);
    }
}

void PowerUpSystem::clear(GameWorld& world)
{
    for (std::unique_ptr<PowerUpEffect>& effect : activeEffects_)
    {
        effect->forceExpire(world);
    }

    activeEffects_.clear();
    droppedPowerUps_.clear();
    syncWorldEffects(world);
}

void PowerUpSystem::activate(PowerUpType type, GameWorld& world)
{
    const auto configIt = configByType_.find(type);
    if (configIt == configByType_.end())
    {
        return;
    }

    for (std::unique_ptr<PowerUpEffect>& activeEffect : activeEffects_)
    {
        if (!activeEffect || activeEffect->isFinished() || activeEffect->type() != type)
        {
            continue;
        }

        if (activeEffect->isPermanent())
        {
            activeEffect->apply(world);
            return;
        }

        activeEffect->extendDuration(configIt->second.durationSeconds);
        activeEffect->apply(world);
        return;
    }

    PowerUpFactory factory;
    std::unique_ptr<PowerUpEffect> effect = factory.create(configIt->second);
    effect->apply(world);
    activeEffects_.push_back(std::move(effect));
}

void PowerUpSystem::cleanupDrops(float offScreenY)
{
    droppedPowerUps_.erase(
        std::remove_if(
            droppedPowerUps_.begin(),
            droppedPowerUps_.end(),
            [offScreenY](const PowerUp& powerUp)
            {
                return powerUp.isCollected() || powerUp.isOffScreen(offScreenY);
            }),
        droppedPowerUps_.end());
}

void PowerUpSystem::cleanupFinishedEffects(GameWorld& world)
{
    activeEffects_.erase(
        std::remove_if(
            activeEffects_.begin(),
            activeEffects_.end(),
            [&world](const std::unique_ptr<PowerUpEffect>& effect)
            {
                if (!effect->isFinished())
                {
                    return false;
                }

                effect->forceExpire(world);
                return true;
            }),
        activeEffects_.end());
}

void PowerUpSystem::syncWorldEffects(GameWorld& world) noexcept
{
    world.activeEffects = &activeEffects_;
}
} // namespace game
