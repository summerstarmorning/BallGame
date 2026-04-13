#include "Particles/ParticleSystem.hpp"

#include <algorithm>
#include <cmath>

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
        reflectOnEdges(particle);
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

void ParticleSystem::setReflectionBounds(const Rect& bounds) noexcept
{
    reflectionBounds_ = bounds;
}

void ParticleSystem::setEdgeReflectionEnabled(bool enabled) noexcept
{
    edgeReflectionEnabled_ = enabled;
}

void ParticleSystem::setReflectionDamping(float damping) noexcept
{
    reflectionDamping_ = std::clamp(damping, 0.0F, 1.0F);
}

void ParticleSystem::emit(const Particle& particle)
{
    if (particles_.size() >= maxParticles_)
    {
        particles_.erase(particles_.begin());
    }

    particles_.push_back(particle);
}

void ParticleSystem::reflectOnEdges(Particle& particle) const noexcept
{
    if (!edgeReflectionEnabled_ || reflectionBounds_.width <= 0.0F || reflectionBounds_.height <= 0.0F)
    {
        return;
    }

    const float left = reflectionBounds_.x;
    const float right = reflectionBounds_.x + reflectionBounds_.width;
    const float top = reflectionBounds_.y;
    const float bottom = reflectionBounds_.y + reflectionBounds_.height;

    if (particle.position.x < left)
    {
        particle.position.x = left;
        particle.velocity.x = std::abs(particle.velocity.x) * reflectionDamping_;
    }
    else if (particle.position.x > right)
    {
        particle.position.x = right;
        particle.velocity.x = -std::abs(particle.velocity.x) * reflectionDamping_;
    }

    if (particle.position.y < top)
    {
        particle.position.y = top;
        particle.velocity.y = std::abs(particle.velocity.y) * reflectionDamping_;
    }
    else if (particle.position.y > bottom)
    {
        particle.position.y = bottom;
        particle.velocity.y = -std::abs(particle.velocity.y) * reflectionDamping_;
    }
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
