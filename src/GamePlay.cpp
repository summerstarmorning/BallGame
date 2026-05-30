#include "Game.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "GameStyle.hpp"

namespace
{
constexpr float kCollisionSlop = 0.01F;
constexpr float kContactEpsilon = 0.0001F;
constexpr float kPaddleRestitution = 1.0F;
constexpr float kDirectionEpsilon = 0.001F;
constexpr float kMinVerticalSpeedRatio = 0.54F;
constexpr float kPaddleHorizontalTransfer = 0.54F;
constexpr float kPaddleVerticalTransfer = 0.22F;

float Dot(const game::Vec2& a, const game::Vec2& b)
{
    return a.x * b.x + a.y * b.y;
}

float LengthSquared(const game::Vec2& value)
{
    return Dot(value, value);
}

float SignOrDefault(float value, float fallback = 1.0F)
{
    if (value > 0.0F)
    {
        return 1.0F;
    }

    if (value < 0.0F)
    {
        return -1.0F;
    }

    return fallback >= 0.0F ? 1.0F : -1.0F;
}

game::Vec2 operator+(const game::Vec2& a, const game::Vec2& b)
{
    return game::Vec2 {a.x + b.x, a.y + b.y};
}

game::Vec2 operator-(const game::Vec2& a, const game::Vec2& b)
{
    return game::Vec2 {a.x - b.x, a.y - b.y};
}

game::Vec2 operator*(const game::Vec2& value, float scalar)
{
    return game::Vec2 {value.x * scalar, value.y * scalar};
}

game::Vec2 ClosestPointOnRect(const game::Vec2& point, const Rectangle& rect)
{
    return game::Vec2 {
        std::clamp(point.x, rect.x, rect.x + rect.width),
        std::clamp(point.y, rect.y, rect.y + rect.height),
    };
}

game::Vec2 ResolveCircleRectNormal(const game::Ball& ball, const Rectangle& rect, float& penetration)
{
    // First use the closest point method, which is enough for the common case
    // where the circle center is outside the rectangle. If the center is inside
    // the rectangle, fall back to the nearest face so the ball can be pushed out
    // along a stable normal instead of jittering between axes.
    const game::Vec2 closestPoint = ClosestPointOnRect(ball.position, rect);
    const game::Vec2 offset = ball.position - closestPoint;
    const float distanceSquared = LengthSquared(offset);

    if (distanceSquared > kContactEpsilon)
    {
        const float distance = std::sqrt(distanceSquared);
        penetration = ball.radius - distance;
        return offset * (1.0F / distance);
    }

    const float distanceToLeft = ball.position.x - rect.x;
    const float distanceToRight = rect.x + rect.width - ball.position.x;
    const float distanceToTop = ball.position.y - rect.y;
    const float distanceToBottom = rect.y + rect.height - ball.position.y;

    penetration = ball.radius + distanceToLeft;
    game::Vec2 normal {-1.0F, 0.0F};

    if (distanceToRight < distanceToLeft)
    {
        penetration = ball.radius + distanceToRight;
        normal = game::Vec2 {1.0F, 0.0F};
    }

    if (distanceToTop < std::min(distanceToLeft, distanceToRight))
    {
        penetration = ball.radius + distanceToTop;
        normal = game::Vec2 {0.0F, -1.0F};
    }

    if (distanceToBottom < std::min({distanceToLeft, distanceToRight, distanceToTop}))
    {
        penetration = ball.radius + distanceToBottom;
        normal = game::Vec2 {0.0F, 1.0F};
    }

    return normal;
}

void ConstrainBallTravelDirection(game::Ball& ball)
{
    // Very flat trajectories can create long boring rallies or near-deadlocks.
    // Keep the total speed, but enforce a minimum vertical component so each
    // rebound continues to make progress through the brick field.
    const float speedSquared = LengthSquared(ball.velocity);
    if (speedSquared <= kDirectionEpsilon)
    {
        return;
    }

    const float speed = std::sqrt(speedSquared);
    const float absVerticalSpeed = std::abs(ball.velocity.y);
    const float minVerticalSpeed = speed * kMinVerticalSpeedRatio;
    if (absVerticalSpeed + kDirectionEpsilon >= minVerticalSpeed)
    {
        return;
    }

    const float constrainedVerticalSpeed = minVerticalSpeed;
    const float constrainedHorizontalSpeed
        = std::sqrt(std::max(speedSquared - constrainedVerticalSpeed * constrainedVerticalSpeed, 0.0F));

    ball.velocity.x = constrainedHorizontalSpeed * SignOrDefault(ball.velocity.x);
    ball.velocity.y = constrainedVerticalSpeed * SignOrDefault(ball.velocity.y, -1.0F);
}

int ChooseBrickDurability(int row)
{
    (void)row;
    return 1;
}

int ChooseBrickShape(int durability)
{
    const int roll = GetRandomValue(0, 99);
    if (durability >= 3)
    {
        return roll < 50 ? 4 : 5;
    }

    if (durability == 2)
    {
        if (roll < 18)
        {
            return 2;
        }

        if (roll < 45)
        {
            return 3;
        }

        if (roll < 73)
        {
            return 4;
        }

        return 5;
    }

    if (roll < 8)
    {
        return 0;
    }

    if (roll < 30)
    {
        return 1;
    }

    if (roll < 50)
    {
        return 2;
    }

    if (roll < 68)
    {
        return 3;
    }

    if (roll < 84)
    {
        return 4;
    }

    return 5;
}
} // namespace

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
    for (const game::BrickRecord& brick : currentLevelData.bricks)
    {
        bricks.emplace_back(brick.x, brick.y, brick.width, brick.height, brick.shape, 1);
    }

    if (!currentLevelData.bricks.empty())
    {
        editorBrickWidth = currentLevelData.bricks.front().width + 8.0F;
        editorBrickHeight = currentLevelData.bricks.front().height + 8.0F;
    }

    AssignPowerUpsToBricks();
    RebuildBrickSpatialGrid();
}

void Game::RebuildBrickSpatialGrid()
{
    // The grid is a broad-phase filter only. It returns possible brick indices;
    // HandleBallBrickCollision still runs raylib's exact circle/rectangle check.
    brickCollisionRects.clear();
    brickCollisionRects.reserve(bricks.size());
    for (const Brick& brick : bricks)
    {
        brickCollisionRects.push_back(brick.GetRect());
    }

    brickCollisionCandidates.clear();
    brickCollisionCandidates.reserve(std::min<std::size_t>(bricks.size(), 24U));
    brickSpatialGrid.rebuild(
        brickCollisionRects,
        Rectangle {0.0F, HUD_HEIGHT, (float)screenWidth, (float)screenHeight - HUD_HEIGHT},
        112.0F,
        52.0F);
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

    const float perBrickDropChance = std::clamp(0.36F + totalWeight * 0.50F, 0.72F, 1.0F);

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
    const int targetMultiBallCount = std::min<int>(8, (int)brickPowerUps.size());
    std::vector<std::size_t> multiBallCandidates;
    multiBallCandidates.reserve(brickPowerUps.size());
    for (std::size_t index = 0; index < brickPowerUps.size(); ++index)
    {
        if (brickPowerUps[index].has_value() && *brickPowerUps[index] == game::PowerUpType::MultiBall)
        {
            continue;
        }
        multiBallCandidates.push_back(index);
    }

    while (multiBallCount < targetMultiBallCount && !multiBallCandidates.empty())
    {
        const int pick = GetRandomValue(0, (int)multiBallCandidates.size() - 1);
        const std::size_t forcedIndex = multiBallCandidates[(std::size_t)pick];
        brickPowerUps[forcedIndex] = game::PowerUpType::MultiBall;
        ++multiBallCount;
        multiBallCandidates.erase(multiBallCandidates.begin() + pick);
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

void Game::HandleBalls(const game::Vec2& paddleVelocity, float deltaSeconds)
{
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
        HandleBallPaddleCollision(managedBall, paddleVelocity);
        HandleBallBrickCollision(managedBall);
        ConstrainBallTravelDirection(managedBall);
        const std::size_t trailCount = 6U
            + (std::size_t)std::min<int>(6, (int)powerUpSystem.activeEffects().size() * 2)
            + (pendingPierceCharges > 0 ? 4U : 0U);
        particleSystem.spawnBallTrail(
            managedBall.position,
            managedBall.velocity,
            game::Color {ballColor.r, ballColor.g, ballColor.b, 220},
            trailCount);
    }

    if (ballManager.consumeLifeLoss())
    {
        if (!debugMode)
        {
            --lives;
            ++playerProfile.totalBallLosses;
        }
        ResetCombo();

        if (lives <= 0 && !debugMode)
        {
            currentState = GameState::GAMEOVER;
            powerUpSystem.clear(world);
            FinalizeRunProgress();
            ClearRuntimeSave();
        }
        else
        {
            powerUpSystem.onLevelTransition(world, true);
            ResetBalls();
            SaveRuntimeProgress();
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

void Game::HandleBallPaddleCollision(game::Ball& managedBall, const game::Vec2& paddleVelocity)
{
    const Rectangle paddleRect = paddle.GetRect();
    if (!CheckCollisionCircleRec(game_style::toRayVec(managedBall.position), managedBall.radius, paddleRect))
    {
        return;
    }

    float penetration = 0.0F;
    const game::Vec2 collisionNormal = ResolveCircleRectNormal(managedBall, paddleRect, penetration);
    if (penetration > 0.0F)
    {
        managedBall.position = managedBall.position + collisionNormal * (penetration + kCollisionSlop);
    }

    const game::Vec2 effectivePaddleVelocity {
        paddleVelocity.x * kPaddleHorizontalTransfer,
        paddleVelocity.y * kPaddleVerticalTransfer,
    };
    // Reflect in the paddle's moving frame. This is the main "free control"
    // upgrade: vertical and horizontal paddle motion inject a small amount of
    // impulse into the rebound, rather than simply flipping the ball velocity.
    const game::Vec2 relativeVelocity = managedBall.velocity - effectivePaddleVelocity;
    const float normalVelocity = Dot(relativeVelocity, collisionNormal);
    if (normalVelocity >= 0.0F)
    {
        return;
    }

    const game::Vec2 reflectedRelativeVelocity
        = relativeVelocity - collisionNormal * ((1.0F + kPaddleRestitution) * normalVelocity);
    managedBall.velocity = reflectedRelativeVelocity + effectivePaddleVelocity;
    TriggerImpact(0.014F, 1.8F);
}

void Game::HandleBallBrickCollision(game::Ball& managedBall)
{
    if (managedBall.collisionResolvedThisFrame)
    {
        return;
    }

    if (!brickSpatialGrid.empty())
    {
        // Broad phase: query only grid cells touched by the ball. This keeps
        // collision cost stable when the level contains many decorative bricks.
        brickSpatialGrid.queryCircle(
            game_style::toRayVec(managedBall.position),
            managedBall.radius,
            brickCollisionCandidates);
    }
    else
    {
        brickCollisionCandidates.clear();
        brickCollisionCandidates.reserve(bricks.size());
        for (std::size_t brickIndex = 0; brickIndex < bricks.size(); ++brickIndex)
        {
            brickCollisionCandidates.push_back(brickIndex);
        }
    }

    for (std::size_t brickIndex : brickCollisionCandidates)
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

        const bool usePierceCharge = pendingPierceCharges > 0;
        const game::Vec2 burstOrigin {brickRect.x + brickRect.width / 2.0F, brickRect.y + brickRect.height / 2.0F};

        if (!usePierceCharge)
        {
            // Normal hits bounce once and mark the ball as resolved for this
            // frame. Pierce hits intentionally skip the bounce and continue
            // through additional bricks while consuming a limited charge count.
            float penetration = 0.0F;
            const game::Vec2 collisionNormal = ResolveCircleRectNormal(managedBall, brickRect, penetration);
            if (penetration > 0.0F)
            {
                managedBall.position = managedBall.position + collisionNormal * (penetration + kCollisionSlop);
            }

            const float normalVelocity = Dot(managedBall.velocity, collisionNormal);
            if (normalVelocity < 0.0F)
            {
                managedBall.velocity = managedBall.velocity - collisionNormal * (2.0F * normalVelocity);
            }
        }

        const int damage = usePierceCharge ? std::max(1, brick.HitPoints()) : 1;
        const bool destroyed = brick.ApplyHit(damage);
        RegisterComboHit(destroyed);
        const int baseScore = destroyed ? 100 * brick.MaxHitPoints() : 45;
        score += destroyed ? baseScore * std::max(1, comboMultiplier) : baseScore;
        if (destroyed)
        {
            RegisterBrickDestroyed(brick.MaxHitPoints());
        }

        if (destroyed)
        {
            hitFlashTimer = std::max(hitFlashTimer, 0.075F);
        }
        TriggerImpact(destroyed ? 0.045F : 0.018F, destroyed ? 12.0F : 4.2F);

        particleSystem.spawnBrickBurst(
            burstOrigin,
            destroyed ? powerUpConfigSet.particles.brickBurstCount + 26U : powerUpConfigSet.particles.brickBurstCount / 2U + 8U);

        if (destroyed)
        {
            particleSystem.spawnBrickBurst(
                burstOrigin,
                powerUpConfigSet.particles.brickBurstCount / 2U + 24U);
            MaybeSpawnPowerUpFromBrick(brickIndex, brickRect);
        }

        if (usePierceCharge)
        {
            pendingPierceCharges = std::max(0, pendingPierceCharges - 1);
            if (pendingPierceCharges <= 0)
            {
                pendingPierceCharges = 0;
            }
            continue;
        }

        managedBall.collisionResolvedThisFrame = true;
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
        levelClearStage = currentLevel;
        levelClearTimer = 1.05F;
        particleSystem.spawnBrickBurst(
            game::Vec2 {(float)screenWidth * 0.5F, HUD_HEIGHT + ((float)screenHeight - HUD_HEIGHT) * 0.35F},
            powerUpConfigSet.particles.brickBurstCount + 42U);
        ++currentLevel;
        ShowNotice(u8"\u5173\u5361\u7a81\u7834\uff0c\u4e0b\u4e00\u9635\u5217\u5df2\u5c55\u5f00", u8"\u5173\u5361\u7a81\u7834", 1.25F);
        edgeParticles.clear();
        LoadLevelForCurrentStage();
        powerUpSystem.onLevelTransition(world, true);
        ResetBalls();
        TriggerImpact(0.035F, 8.5F);
        SaveRuntimeProgress();
        return;
    }

    victory = true;
    FinalizeRunProgress();
    ClearRuntimeSave();
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
        const float normalSpeed = 150.0F + (float)GetRandomValue(0, 150);
        const float tangentSpeed = (float)GetRandomValue(-120, 120);
        const float maxLife = 0.18F + (float)GetRandomValue(0, 28) / 100.0F;
        const float size = 2.0F + (float)GetRandomValue(0, 24) / 10.0F;

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

void Game::TriggerImpact(float freezeSeconds, float shakeMagnitude)
{
    impactFreezeTimer = std::max(impactFreezeTimer, std::clamp(freezeSeconds, 0.0F, 0.09F));
    screenShakeTimer = std::max(screenShakeTimer, std::clamp(freezeSeconds * 4.6F, 0.0F, 0.30F));
    screenShakeMagnitude = std::max(screenShakeMagnitude, std::clamp(shakeMagnitude, 0.0F, 14.0F));
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
