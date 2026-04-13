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
    void setReflectionBounds(const Rect& bounds) noexcept;
    void setEdgeReflectionEnabled(bool enabled) noexcept;
    void setReflectionDamping(float damping) noexcept;

    const std::vector<Particle>& particles() const noexcept { return particles_; }

private:
    void emit(const Particle& particle);
    void reflectOnEdges(Particle& particle) const noexcept;
    void removeExpired();

    std::vector<Particle> particles_ {};
    std::size_t maxParticles_ {512U};
    Rect reflectionBounds_ {};
    float reflectionDamping_ {0.82F};
    bool edgeReflectionEnabled_ {false};
};
} // namespace game
