#include "Effects/MultiBallEffect.hpp"

#include <algorithm>
#include <cmath>
#include <random>

#include "BallManager.hpp"
#include "GameWorld.hpp"

namespace game
{
namespace
{
Vec2 rotate(const Vec2& value, float degrees)
{
    const float rad = degrees * 3.1415926535F / 180.0F;
    const float cosValue = std::cos(rad);
    const float sinValue = std::sin(rad);
    return Vec2 {
        value.x * cosValue - value.y * sinValue,
        value.x * sinValue + value.y * cosValue,
    };
}

int randomIntInclusive(int minValue, int maxValue)
{
    static thread_local std::mt19937 generator {std::random_device {}()};
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(generator);
}
} // namespace

void MultiBallEffect::apply(GameWorld& world)
{
    if (world.ballManager == nullptr || world.ballManager->balls().empty())
    {
        return;
    }

    std::vector<Ball>& balls = world.ballManager->balls();
    const auto sourceIt = std::find_if(
        balls.begin(),
        balls.end(),
        [](const Ball& ball)
        {
            return ball.active;
        });

    if (sourceIt == balls.end())
    {
        return;
    }

    const Ball source = *sourceIt;
    const float speed = std::sqrt(source.velocity.x * source.velocity.x + source.velocity.y * source.velocity.y);
    const Vec2 baseDir = speed > 0.001F ? Vec2 {source.velocity.x / speed, source.velocity.y / speed} : Vec2 {0.0F, -1.0F};
    const float targetSpeed = speed > 0.001F ? speed * 1.05F : 260.0F;
    const float splitRadius = std::max(12.0F, source.radius * 0.94F);
    const int splitCount = randomIntInclusive(3, 5); // total balls after split
    const float minAngle = -42.0F;
    const float maxAngle = 42.0F;

    sourceIt->radius = splitRadius;
    sourceIt->velocity = Vec2 {baseDir.x * targetSpeed, baseDir.y * targetSpeed};

    for (int index = 0; index < splitCount; ++index)
    {
        const float ratio = splitCount == 1 ? 0.5F : (float)index / (float)(splitCount - 1);
        const float angle = minAngle + (maxAngle - minAngle) * ratio;
        const Vec2 dir = rotate(baseDir, angle);
        if (index == splitCount / 2)
        {
            sourceIt->velocity = Vec2 {dir.x * targetSpeed, dir.y * targetSpeed};
            sourceIt->position = Vec2 {
                source.position.x + dir.x * (splitRadius + 3.0F),
                source.position.y + dir.y * (splitRadius + 3.0F),
            };
            sourceIt->collisionResolvedThisFrame = false;
            continue;
        }

        Ball clone = source;
        clone.radius = splitRadius;
        clone.velocity = Vec2 {dir.x * targetSpeed, dir.y * targetSpeed};
        clone.position = Vec2 {
            source.position.x + dir.x * (splitRadius + 5.0F),
            source.position.y + dir.y * (splitRadius + 5.0F),
        };
        clone.active = true;
        clone.collisionResolvedThisFrame = false;
        world.ballManager->addBall(clone);
    }
}

void MultiBallEffect::expire(GameWorld& world)
{
    (void)world;
}
} // namespace game
