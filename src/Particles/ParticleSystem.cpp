#include "Particles/ParticleSystem.hpp"

#include <algorithm>

namespace game
{
ParticleSystem::ParticleSystem(std::size_t maxParticles)
    : maxParticles_(maxParticles)
{
}

void ParticleSystem::spawnBrickBurst(const Vec2& origin, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i)
    {
        emit(Particle {origin, Vec2 {0.0F, -40.0F}, Color {255, 180, 90, 255}, 0.5F, 0.0F, true});
    }
}

void ParticleSystem::spawnPowerUpTrail(const Vec2& origin, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i)
    {
        emit(Particle {origin, Vec2 {0.0F, 10.0F}, Color {120, 220, 255, 180}, 0.25F, 0.0F, true});
    }
}

void ParticleSystem::update(float deltaSeconds)
{
    for (Particle& particle : particles_)
    {
        particle.ageSeconds += deltaSeconds;
        particle.position.x += particle.velocity.x * deltaSeconds;
        particle.position.y += particle.velocity.y * deltaSeconds;
        particle.active = particle.ageSeconds < particle.lifetimeSeconds;
    }

    removeExpired();
}

void ParticleSystem::render() const
{
    // Rendering should be delegated to the engine adapter.
}

void ParticleSystem::clear() noexcept
{
    particles_.clear();
}

void ParticleSystem::emit(const Particle& particle)
{
    if (particles_.size() >= maxParticles_)
    {
        particles_.erase(particles_.begin());
    }

    particles_.push_back(particle);
}

void ParticleSystem::removeExpired()
{
    particles_.erase(
        std::remove_if(
            particles_.begin(),
            particles_.end(),
            [](const Particle& particle)
            {
                return !particle.active;
            }),
        particles_.end());
}
} // namespace game
