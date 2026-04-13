#pragma once

#include <vector>

#include "GameTypes.hpp"

namespace game
{
struct Particle
{
    Vec2 position {};
    Vec2 velocity {};
    Color color {};
    float lifetimeSeconds {0.0F};
    float ageSeconds {0.0F};
    bool active {true};
};

class ParticleSystem
{
public:
    explicit ParticleSystem(std::size_t maxParticles = 512U);

    void spawnBrickBurst(const Vec2& origin, std::size_t count);
    void spawnPowerUpTrail(const Vec2& origin, std::size_t count);
    void update(float deltaSeconds);
    void render() const;
    void clear() noexcept;

    const std::vector<Particle>& particles() const noexcept { return particles_; }

private:
    void emit(const Particle& particle);
    void removeExpired();

    std::vector<Particle> particles_ {};
    std::size_t maxParticles_ {512U};
};
} // namespace game
