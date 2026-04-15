#include "Particles/ParticleSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace game
{
namespace
{
float randomFloat(float minValue, float maxValue)
{
    static thread_local std::mt19937 generator {std::random_device {}()};
    std::uniform_real_distribution<float> distribution(minValue, maxValue);
    return distribution(generator);
}

int randomInt(int minValue, int maxValue)
{
    static thread_local std::mt19937 generator {std::random_device {}()};
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}

const std::array<Color, 6>& burstPalette()
{
    static const std::array<Color, 6> palette {
        Color {255, 120, 56, 255},
        Color {255, 180, 74, 255},
        Color {255, 224, 110, 255},
        Color {123, 208, 255, 255},
        Color {255, 98, 168, 255},
        Color {168, 255, 216, 255},
    };
    return palette;
}
} // namespace

ParticleSystem::ParticleSystem(std::size_t maxParticles)
    : maxParticles_(maxParticles)
{
}

void ParticleSystem::spawnBrickBurst(const Vec2& origin, std::size_t count)
{
    if (count == 0U)
    {
        return;
    }

    const auto& palette = burstPalette();
    for (std::size_t i = 0; i < count; ++i)
    {
        const float angle = randomFloat(-3.1415926F, 3.1415926F);
        const float speed = randomFloat(120.0F, 340.0F);
        Vec2 velocity {
            std::cos(angle) * speed,
            std::sin(angle) * speed - randomFloat(35.0F, 130.0F),
        };

        Color burstColor = palette[(std::size_t)randomInt(0, (int)palette.size() - 1)];
        burstColor.a = (unsigned char)randomInt(170, 255);
        emit(Particle {origin, velocity, burstColor, randomFloat(0.28F, 0.86F), 0.0F, true});
    }

    const std::size_t sparkleCount = std::max<std::size_t>(3U, count / 3U);
    for (std::size_t i = 0; i < sparkleCount; ++i)
    {
        const float angle = randomFloat(-3.1415926F, 3.1415926F);
        const float speed = randomFloat(220.0F, 460.0F);
        const Color sparkleColor = randomInt(0, 1) == 0 ? Color {255, 255, 255, 240} : Color {255, 238, 145, 230};
        emit(Particle {
            origin,
            Vec2 {std::cos(angle) * speed, std::sin(angle) * speed},
            sparkleColor,
            randomFloat(0.16F, 0.36F),
            0.0F,
            true,
        });
    }
}

void ParticleSystem::spawnPowerUpTrail(const Vec2& origin, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i)
    {
        emit(Particle {
            Vec2 {origin.x + randomFloat(-4.0F, 4.0F), origin.y + randomFloat(-3.0F, 3.0F)},
            Vec2 {randomFloat(-26.0F, 26.0F), randomFloat(18.0F, 76.0F)},
            Color {(unsigned char)randomInt(110, 180), (unsigned char)randomInt(210, 255), 255, (unsigned char)randomInt(130, 210)},
            randomFloat(0.22F, 0.40F),
            0.0F,
            true,
        });
    }
}

void ParticleSystem::spawnBallTrail(const Vec2& origin, const Vec2& velocity, const Color& tint, std::size_t count)
{
    if (count == 0U)
    {
        return;
    }

    const float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    const float invLen = speed > 0.001F ? 1.0F / speed : 0.0F;
    const Vec2 dir {velocity.x * invLen, velocity.y * invLen};
    const Vec2 side {-dir.y, dir.x};

    for (std::size_t i = 0; i < count; ++i)
    {
        const float spread = randomFloat(-1.0F, 1.0F);
        const float backSpeed = randomFloat(58.0F, 122.0F) + speed * 0.08F;
        const float sideSpeed = randomFloat(-38.0F, 38.0F);

        Color c = tint;
        c.a = (unsigned char)randomInt(110, 190);

        emit(Particle {
            Vec2 {
                origin.x - dir.x * randomFloat(2.0F, 10.0F) + side.x * spread * 4.0F,
                origin.y - dir.y * randomFloat(2.0F, 10.0F) + side.y * spread * 4.0F,
            },
            Vec2 {
                -dir.x * backSpeed + side.x * sideSpeed,
                -dir.y * backSpeed + side.y * sideSpeed,
            },
            c,
            randomFloat(0.10F, 0.24F),
            0.0F,
            true,
        });
    }
}

void ParticleSystem::update(float deltaSeconds)
{
    for (Particle& particle : particles_)
    {
        if (!particle.active)
        {
            continue;
        }

        particle.ageSeconds += deltaSeconds;
        particle.velocity.y += 240.0F * deltaSeconds;
        particle.velocity.x *= std::clamp(1.0F - deltaSeconds * 1.6F, 0.85F, 1.0F);
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
