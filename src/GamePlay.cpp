#include "Game.h"

#include <algorithm>
#include <cmath>

#include "GameStyle.hpp"

float Game::PaddleMinY() const
{
    return std::clamp((float)screenHeight * 0.56F, HUD_HEIGHT + 22.0F, (float)screenHeight - 180.0F);
}

float Game::PaddleMaxY() const
{
    return (float)screenHeight - 12.0F;
}

void Game::InitBricks()
{
    bricks.clear();

    const float sidePadding = 52.0F;
    const float topStart = HUD_HEIGHT + 56.0F;
    constexpr int rows = 7;
    constexpr float rowGap = 44.0F;
    const float rightLimit = (float)screenWidth - sidePadding;

    for (int row = 0; row < rows; ++row)
    {
        float cursorX = sidePadding + (float)GetRandomValue(-18, 16);
        const float rowBaseY = topStart + (float)row * rowGap + (float)GetRandomValue(-6, 8);

        while (cursorX < rightLimit - 72.0F)
        {
            const float width = (float)GetRandomValue(62, 168);
            const float height = (float)GetRandomValue(20, 36);
            const float y = rowBaseY + (float)GetRandomValue(-6, 6);

            if (cursorX + width > rightLimit)
            {
                break;
            }

            const int shapeType = GetRandomValue(0, 3);
            bricks.emplace_back(cursorX, y, width, height, shapeType);

            const float gap = (float)GetRandomValue(10, 26);
            const bool skipSegment = GetRandomValue(0, 100) < 15;
            cursorX += width + gap + (skipSegment ? (float)GetRandomValue(24, 66) : 0.0F);
        }
    }

    AssignPowerUpsToBricks();
}

void Game::AssignPowerUpsToBricks()
{
    brickPowerUps.assign(bricks.size(), std::nullopt);
    if (bricks.empty() || powerUpConfigSet.powerUps.empty())
    {
        return;
    }

    std::vector<std::pair<game::PowerUpType, float>> weightedTypes;
    weightedTypes.reserve(powerUpConfigSet.powerUps.size());

    float totalWeight = 0.0F;
    for (const auto& [type, config] : powerUpConfigSet.powerUps)
    {
        if (config.dropChance <= 0.0F)
        {
            continue;
        }

        weightedTypes.emplace_back(type, config.dropChance);
        totalWeight += config.dropChance;
    }

    if (weightedTypes.empty() || totalWeight <= 0.0001F)
    {
        return;
    }

    const float perBrickDropChance = std::clamp(0.14F + totalWeight * 0.36F, 0.30F, 0.86F);

    for (std::size_t index = 0; index < brickPowerUps.size(); ++index)
    {
        const float spawnRoll = (float)GetRandomValue(0, 10000) / 10000.0F;
        if (spawnRoll > perBrickDropChance)
        {
            continue;
        }

        const float pickRoll = ((float)GetRandomValue(0, 10000) / 10000.0F) * totalWeight;
        float cumulative = 0.0F;
        for (const auto& [type, weight] : weightedTypes)
        {
            cumulative += weight;
            if (pickRoll <= cumulative)
            {
                brickPowerUps[index] = type;
                break;
            }
        }
    }

    // Ensure every configured power-up type appears at least once per level.
    for (const auto& [type, _] : weightedTypes)
    {
        const bool alreadyAssigned = std::any_of(
            brickPowerUps.begin(),
            brickPowerUps.end(),
            [type](const std::optional<game::PowerUpType>& value)
            {
                return value.has_value() && *value == type;
            });

        if (alreadyAssigned)
        {
            continue;
        }

        const std::size_t forcedIndex = (std::size_t)GetRandomValue(0, (int)brickPowerUps.size() - 1);
        brickPowerUps[forcedIndex] = type;
    }

    int multiBallCount = (int)std::count_if(
        brickPowerUps.begin(),
        brickPowerUps.end(),
        [](const std::optional<game::PowerUpType>& value)
        {
            return value.has_value() && *value == game::PowerUpType::MultiBall;
        });
    while (multiBallCount < 3)
    {
        const std::size_t forcedIndex = (std::size_t)GetRandomValue(0, (int)brickPowerUps.size() - 1);
        if (brickPowerUps[forcedIndex].has_value() && *brickPowerUps[forcedIndex] == game::PowerUpType::MultiBall)
        {
            continue;
        }

        brickPowerUps[forcedIndex] = game::PowerUpType::MultiBall;
        ++multiBallCount;
    }
}

void Game::ResetBalls()
{
    ballManager = game::BallManager {};
    world.ballManager = &ballManager;

    game::Ball openingBall {};
    openingBall.position = game_style::toGameVec(spawnBallPosition);
    openingBall.velocity = game_style::toGameVec(spawnBallVelocity);
    openingBall.radius = spawnBallRadius;
    openingBall.active = true;
    openingBall.collisionResolvedThisFrame = false;
    ballManager.addBall(openingBall);
}

void Game::SyncEffectPaddleToGameplay()
{
    if (!effectPaddle)
    {
        effectPaddle = std::make_unique<game::Paddle>(game_style::toGameRect(paddle.GetRect()));
    }

    game::Rect effectBounds = effectPaddle->bounds();
    const Rectangle gameplayBounds = paddle.GetRect();
    effectBounds.x = gameplayBounds.x;
    effectBounds.y = gameplayBounds.y;
    effectBounds.height = gameplayBounds.height;

    if (effectBounds.width <= 0.0F)
    {
        effectBounds.width = gameplayBounds.width;
    }

    effectPaddle->setBounds(effectBounds);
    world.paddle = effectPaddle.get();
}

void Game::ApplyEffectPaddleToGameplay()
{
    if (!effectPaddle)
    {
        return;
    }

    Rectangle gameplayBounds = game_style::toRayRect(effectPaddle->bounds());
    if (gameplayBounds.x < WALL_THICKNESS)
    {
        gameplayBounds.x = WALL_THICKNESS;
    }

    const float maxRight = (float)screenWidth - WALL_THICKNESS;
    if (gameplayBounds.x + gameplayBounds.width > maxRight)
    {
        gameplayBounds.x = maxRight - gameplayBounds.width;
    }

    const float minY = PaddleMinY();
    const float maxY = PaddleMaxY();
    if (gameplayBounds.y < minY)
    {
        gameplayBounds.y = minY;
    }

    if (gameplayBounds.y + gameplayBounds.height > maxY)
    {
        gameplayBounds.y = maxY - gameplayBounds.height;
    }

    paddle.SetRect(gameplayBounds);
}

void Game::HandleBalls(float paddleVel)
{
    const float deltaSeconds = std::max(GetFrameTime(), 0.0001F);
    const game::Rect playBounds {
        WALL_THICKNESS,
        WALL_THICKNESS,
        (float)screenWidth - WALL_THICKNESS * 2.0F,
        (float)screenHeight - WALL_THICKNESS,
    };

    ballManager.update(deltaSeconds, playBounds);

    for (game::Ball& managedBall : ballManager.balls())
    {
        if (!managedBall.active)
        {
            continue;
        }

        HandleBallEdgeCollision(managedBall);
        HandleBallPaddleCollision(managedBall, paddleVel);
        HandleBallBrickCollision(managedBall);
        particleSystem.spawnBallTrail(
            managedBall.position,
            managedBall.velocity,
            game::Color {ballColor.r, ballColor.g, ballColor.b, 220},
            3U);
    }

    if (ballManager.consumeLifeLoss())
    {
        if (!debugMode)
        {
            --lives;
        }

        if (lives <= 0 && !debugMode)
        {
            currentState = GameState::GAMEOVER;
            powerUpSystem.clear(world);
        }
        else
        {
            powerUpSystem.onLevelTransition(world, true);
            ResetBalls();
        }
    }

    CheckLevelProgress();
}

void Game::HandleBallEdgeCollision(game::Ball& managedBall)
{
    const float left = WALL_THICKNESS + managedBall.radius;
    const float right = (float)screenWidth - WALL_THICKNESS - managedBall.radius;
    const float top = WALL_THICKNESS + managedBall.radius;
    const float contactPadding = managedBall.radius + 2.0F;

    if (managedBall.position.x < left)
    {
        managedBall.position.x = left;
        if (managedBall.velocity.x < 0.0F)
        {
            managedBall.velocity.x = -managedBall.velocity.x;
            SpawnEdgeParticles(
                Vector2 {contactPadding, managedBall.position.y},
                Vector2 {1.0F, 0.0F},
                12);
        }
    }
    else if (managedBall.position.x > right)
    {
        managedBall.position.x = right;
        if (managedBall.velocity.x > 0.0F)
        {
            managedBall.velocity.x = -managedBall.velocity.x;
            SpawnEdgeParticles(
                Vector2 {(float)screenWidth - contactPadding, managedBall.position.y},
                Vector2 {-1.0F, 0.0F},
                12);
        }
    }

    if (managedBall.position.y < top)
    {
        managedBall.position.y = top;
        if (managedBall.velocity.y < 0.0F)
        {
            managedBall.velocity.y = -managedBall.velocity.y;
            SpawnEdgeParticles(Vector2 {managedBall.position.x, contactPadding}, Vector2 {0.0F, 1.0F}, 12);
        }
    }
}

void Game::HandleBallPaddleCollision(game::Ball& managedBall, float paddleVel)
{
    const Rectangle paddleRect = paddle.GetRect();
    if (!CheckCollisionCircleRec(game_style::toRayVec(managedBall.position), managedBall.radius, paddleRect))
    {
        return;
    }

    if (managedBall.velocity.y <= 0.0F)
    {
        return;
    }

    managedBall.position.y = paddleRect.y - managedBall.radius - 1.0F;
    managedBall.velocity.y = -std::abs(managedBall.velocity.y);
    managedBall.velocity.x += paddleVel * PADDLE_INFLUENCE * 60.0F;

    const float maxHorizontalSpeed = MAX_H_SPEED * 60.0F;
    managedBall.velocity.x = std::clamp(managedBall.velocity.x, -maxHorizontalSpeed, maxHorizontalSpeed);
}

void Game::HandleBallBrickCollision(game::Ball& managedBall)
{
    if (managedBall.collisionResolvedThisFrame)
    {
        return;
    }

    for (std::size_t brickIndex = 0; brickIndex < bricks.size(); ++brickIndex)
    {
        Brick& brick = bricks[brickIndex];
        if (!brick.IsActive())
        {
            continue;
        }

        const Rectangle brickRect = brick.GetRect();
        if (!CheckCollisionCircleRec(game_style::toRayVec(managedBall.position), managedBall.radius, brickRect))
        {
            continue;
        }

        const bool hitSide
            = (managedBall.position.x < brickRect.x) || (managedBall.position.x > brickRect.x + brickRect.width);
        const bool hitTopBottom
            = (managedBall.position.y < brickRect.y) || (managedBall.position.y > brickRect.y + brickRect.height);

        if (hitSide && !hitTopBottom)
        {
            managedBall.velocity.x = -managedBall.velocity.x;
        }
        else if (!hitSide && hitTopBottom)
        {
            managedBall.velocity.y = -managedBall.velocity.y;
        }
        else
        {
            managedBall.velocity.x = -managedBall.velocity.x;
            managedBall.velocity.y = -managedBall.velocity.y;
        }

        brick.SetActive(false);
        score += 100;
        managedBall.collisionResolvedThisFrame = true;

        particleSystem.spawnBrickBurst(
            game::Vec2 {brickRect.x + brickRect.width / 2.0F, brickRect.y + brickRect.height / 2.0F},
            powerUpConfigSet.particles.brickBurstCount);
        particleSystem.spawnBrickBurst(
            game::Vec2 {brickRect.x + brickRect.width / 2.0F, brickRect.y + brickRect.height / 2.0F},
            powerUpConfigSet.particles.brickBurstCount / 3U + 6U);

        MaybeSpawnPowerUpFromBrick(brickIndex, brickRect);
        return;
    }
}

void Game::CheckLevelProgress()
{
    const bool allBricksCleared = std::none_of(
        bricks.begin(),
        bricks.end(),
        [](const Brick& brick)
        {
            return brick.IsActive();
        });

    if (!allBricksCleared)
    {
        return;
    }

    const auto& levels = game_style::levelConfigs();
    if (currentLevel < (int)levels.size())
    {
        ++currentLevel;
        particleSystem.clear();
        edgeParticles.clear();
        InitConfigAndBricks(levels[(std::size_t)currentLevel - 1U]);
        powerUpSystem.onLevelTransition(world, true);
        ResetBalls();
        return;
    }

    victory = true;
}

void Game::MaybeSpawnPowerUpFromBrick(std::size_t brickIndex, const Rectangle& brickRect)
{
    if (brickIndex >= brickPowerUps.size() || !brickPowerUps[brickIndex].has_value())
    {
        return;
    }

    const game::Vec2 spawnPosition {
        brickRect.x + brickRect.width * 0.5F - 12.0F,
        brickRect.y + brickRect.height * 0.5F - 6.0F,
    };
    const game::PowerUpType type = *brickPowerUps[brickIndex];
    (void)powerUpSystem.trySpawnFromBrickBreak(type, spawnPosition, 0.0F);
    brickPowerUps[brickIndex] = std::nullopt;
}

Color Game::GetBrickDisplayColor(std::size_t brickIndex, Color baseColor, float timeSeconds, float globalGlow) const
{
    Color result = baseColor;
    if (brickIndex < brickPowerUps.size() && brickPowerUps[brickIndex].has_value())
    {
        const Color powerColor = game_style::powerUpColor(*brickPowerUps[brickIndex]);
        const float pulse = 0.42F + 0.22F * std::sin(timeSeconds * 4.8F + (float)brickIndex * 0.24F);
        result = game_style::mixColor(result, powerColor, std::clamp(pulse, 0.28F, 0.72F));
    }

    if (globalGlow > 0.0F)
    {
        result = game_style::mixColor(result, Color {255, 255, 255, 255}, std::clamp(globalGlow * 0.08F, 0.0F, 0.15F));
    }

    return result;
}

void Game::SpawnEdgeParticles(const Vector2& origin, const Vector2& normal, int count)
{
    if (count <= 0)
    {
        return;
    }

    const Vector2 tangent {-normal.y, normal.x};
    for (int index = 0; index < count; ++index)
    {
        const float normalSpeed = 100.0F + (float)GetRandomValue(0, 90);
        const float tangentSpeed = (float)GetRandomValue(-70, 70);
        const float maxLife = 0.30F + (float)GetRandomValue(0, 55) / 100.0F;
        const float size = 1.8F + (float)GetRandomValue(0, 18) / 10.0F;

        EdgeParticle particle {};
        particle.position = origin;
        particle.velocity = Vector2 {
            normal.x * normalSpeed + tangent.x * tangentSpeed,
            normal.y * normalSpeed + tangent.y * tangentSpeed,
        };
        particle.life = maxLife;
        particle.maxLife = maxLife;
        particle.size = size;
        particle.color = ballColor;
        edgeParticles.push_back(particle);
    }

    if ((int)edgeParticles.size() > MAX_EDGE_PARTICLES)
    {
        const std::size_t overflow = edgeParticles.size() - (std::size_t)MAX_EDGE_PARTICLES;
        edgeParticles.erase(edgeParticles.begin(), edgeParticles.begin() + overflow);
    }
}

void Game::UpdateEdgeParticles()
{
    const float deltaSeconds = std::max(GetFrameTime(), 0.0001F);
    const float left = WALL_THICKNESS;
    const float right = (float)screenWidth - WALL_THICKNESS;
    const float top = WALL_THICKNESS;
    const float bottom = (float)screenHeight - WALL_THICKNESS;
    constexpr float damping = 0.78F;

    for (EdgeParticle& particle : edgeParticles)
    {
        particle.life -= deltaSeconds;
        particle.position.x += particle.velocity.x * deltaSeconds;
        particle.position.y += particle.velocity.y * deltaSeconds;
        particle.velocity.y += 95.0F * deltaSeconds;

        if (particle.position.x < left)
        {
            particle.position.x = left;
            if (particle.velocity.x < 0.0F)
            {
                particle.velocity.x = -particle.velocity.x * damping;
            }
        }
        else if (particle.position.x > right)
        {
            particle.position.x = right;
            if (particle.velocity.x > 0.0F)
            {
                particle.velocity.x = -particle.velocity.x * damping;
            }
        }

        if (particle.position.y < top)
        {
            particle.position.y = top;
            if (particle.velocity.y < 0.0F)
            {
                particle.velocity.y = -particle.velocity.y * damping;
            }
        }
        else if (particle.position.y > bottom)
        {
            particle.position.y = bottom;
            if (particle.velocity.y > 0.0F)
            {
                particle.velocity.y = -particle.velocity.y * damping;
            }
        }
    }

    edgeParticles.erase(
        std::remove_if(
            edgeParticles.begin(),
            edgeParticles.end(),
            [](const EdgeParticle& particle)
            {
                return particle.life <= 0.0F;
            }),
        edgeParticles.end());
}
