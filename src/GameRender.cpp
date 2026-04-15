#include "Game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Effects/PowerUpEffect.hpp"
#include "GameStyle.hpp"

namespace
{
Color lerpSequence(
    Color baseColor,
    const std::vector<std::unique_ptr<game::PowerUpEffect>>& effects,
    float timeSeconds,
    float* glowStrength)
{
    Color mixed = baseColor;
    float accumulatedGlow = 0.0F;
    int pulseIndex = 0;

    for (const std::unique_ptr<game::PowerUpEffect>& effect : effects)
    {
        if (!effect || effect->isFinished())
        {
            continue;
        }

        const Color tint = game_style::powerUpColor(effect->type());
        float pulse = effect->isPermanent() ? 0.16F : 0.24F + 0.12F * std::sin(timeSeconds * 5.2F + pulseIndex * 0.9F);
        pulse = std::clamp(pulse, 0.06F, 0.45F);
        mixed = game_style::mixColor(mixed, tint, pulse);
        accumulatedGlow = std::clamp(accumulatedGlow + pulse * 0.72F, 0.0F, 0.95F);
        ++pulseIndex;
    }

    if (glowStrength != nullptr)
    {
        *glowStrength = accumulatedGlow;
    }

    return game_style::mixColor(baseColor, mixed, std::clamp(0.35F + accumulatedGlow * 0.45F, 0.0F, 0.9F));
}

void DrawBallTrail(const game::Ball& managedBall, Color ballColor, float timeSeconds)
{
    const float vx = managedBall.velocity.x;
    const float vy = managedBall.velocity.y;
    const float speed = std::sqrt(vx * vx + vy * vy);
    if (speed < 0.001F)
    {
        return;
    }

    const Vector2 center = game_style::toRayVec(managedBall.position);
    const Vector2 dir {vx / speed, vy / speed};
    const Vector2 normal {-dir.y, dir.x};
    const float speedFactor = std::clamp(speed / 540.0F, 0.45F, 1.35F);
    const int steps = (int)std::clamp(5.0F + speedFactor * 3.5F, 5.0F, 9.0F);
    const float stride = managedBall.radius * (0.92F + speedFactor * 0.56F);

    DrawCircleV(center, managedBall.radius * (1.34F + speedFactor * 0.10F), ColorAlpha(ballColor, 0.10F));

    for (int step = 1; step <= steps; ++step)
    {
        const float ratio = (float)step / (float)steps;
        const float fade = 1.0F - ratio;
        const float sway = std::sin(
            timeSeconds * (12.0F + speedFactor * 4.5F) + (float)step * 0.75F + center.x * 0.008F + center.y * 0.006F);
        const float distance = stride * (0.84F + ratio * 0.26F) * (float)step;
        const float sideOffset = sway * managedBall.radius * (0.18F + ratio * 0.36F) * speedFactor;

        const Vector2 trailPos {
            center.x - dir.x * distance + normal.x * sideOffset,
            center.y - dir.y * distance + normal.y * sideOffset,
        };

        const float coreRadius = std::max(2.2F, managedBall.radius * (0.86F - ratio * 0.56F));
        const float blurRadius = coreRadius * (1.48F + ratio * 0.42F);
        const float blurAlpha = std::clamp(0.21F * fade * speedFactor * 0.44F, 0.02F, 0.20F);
        const float coreAlpha = std::clamp(0.25F * fade + 0.03F, 0.03F, 0.30F);
        const float sparkle = 0.90F + 0.10F * std::sin(timeSeconds * 20.0F + (float)step * 1.6F);

        DrawCircleV(trailPos, blurRadius, ColorAlpha(ballColor, blurAlpha));
        DrawCircleV(trailPos, coreRadius, ColorAlpha(ballColor, coreAlpha * sparkle));
    }
}

void DrawTextureCover(const Texture2D& texture, int targetWidth, int targetHeight, float alpha)
{
    if (texture.id == 0 || texture.width <= 0 || texture.height <= 0)
    {
        return;
    }

    const float scale = std::max((float)targetWidth / (float)texture.width, (float)targetHeight / (float)texture.height);
    const float srcWidth = (float)targetWidth / scale;
    const float srcHeight = (float)targetHeight / scale;
    const float srcX = ((float)texture.width - srcWidth) * 0.5F;
    const float srcY = ((float)texture.height - srcHeight) * 0.5F;

    const Rectangle source {srcX, srcY, srcWidth, srcHeight};
    const Rectangle destination {0.0F, 0.0F, (float)targetWidth, (float)targetHeight};
    DrawTexturePro(texture, source, destination, Vector2 {0.0F, 0.0F}, 0.0F, ColorAlpha(WHITE, std::clamp(alpha, 0.0F, 1.0F)));
}
} // namespace

void Game::DrawLocalized(const char* zhText, const char* enText, float x, float y, float fontSize, Color color) const
{
    if (hasUiFont)
    {
        DrawTextEx(uiFont, zhText, Vector2 {x, y}, fontSize, 0.0F, color);
        return;
    }

    DrawText(enText, (int)x, (int)y, (int)fontSize, color);
}

void Game::DrawLocalizedf(
    const char* zhPrefix,
    const char* enPrefix,
    int value,
    float x,
    float y,
    float fontSize,
    Color color) const
{
    char buffer[96] {};
    if (hasUiFont)
    {
        std::snprintf(buffer, sizeof(buffer), "%s %d", zhPrefix, value);
        DrawTextEx(uiFont, buffer, Vector2 {x, y}, fontSize, 0.0F, color);
        return;
    }

    std::snprintf(buffer, sizeof(buffer), "%s %d", enPrefix, value);
    DrawText(buffer, (int)x, (int)y, (int)fontSize, color);
}

void Game::DrawEdgeParticles() const
{
    for (const EdgeParticle& particle : edgeParticles)
    {
        if (particle.maxLife <= 0.0F)
        {
            continue;
        }

        const float lifeRatio = std::clamp(particle.life / particle.maxLife, 0.0F, 1.0F);
        Color drawColor = particle.color;
        drawColor.a = (unsigned char)(230.0F * lifeRatio);
        DrawCircleV(particle.position, particle.size * (0.45F + 0.55F * lifeRatio), drawColor);
    }
}

void Game::DrawPowerUps() const
{
    const float timeSeconds = (float)GetTime();
    for (const game::PowerUp& powerUp : powerUpSystem.droppedPowerUps())
    {
        if (powerUp.isCollected())
        {
            continue;
        }

        Rectangle bounds = game_style::toRayRect(powerUp.bounds());
        bounds.width = std::max(bounds.width, 24.0F);
        bounds.height = std::max(bounds.height, 12.0F);
        const Color fill = game_style::powerUpColor(powerUp.type());
        const float glowPulse = 0.45F + 0.35F * std::sin(timeSeconds * 6.5F + bounds.x * 0.01F);
        DrawRectangleRounded(bounds, 0.35F, 4, ColorAlpha(fill, 0.9F));
        DrawRectangleRounded(bounds, 0.35F, 4, ColorAlpha(fill, std::clamp(0.12F + glowPulse * 0.15F, 0.12F, 0.35F)));
        DrawRectangleRoundedLines(bounds, 0.35F, 4, 2.0F, ColorAlpha(BLACK, 0.25F));
        DrawText(
            game_style::powerUpGlyph(powerUp.type()),
            (int)(bounds.x + bounds.width * 0.5F - 5.0F),
            (int)(bounds.y - 2.0F),
            16,
            ColorAlpha(WHITE, 0.92F));
    }
}

void Game::DrawParticles() const
{
    for (const game::Particle& particle : particleSystem.particles())
    {
        if (!particle.active || particle.lifetimeSeconds <= 0.0F)
        {
            continue;
        }

        const float lifeRatio = std::clamp(1.0F - particle.ageSeconds / particle.lifetimeSeconds, 0.0F, 1.0F);
        Color drawColor = game_style::toRayColor(particle.color);
        drawColor.a = (unsigned char)((float)drawColor.a * lifeRatio);
        const Vector2 position = game_style::toRayVec(particle.position);
        const float radius = 1.0F + 2.6F * lifeRatio;
        DrawCircleV(position, radius * 1.7F, ColorAlpha(drawColor, 0.16F * lifeRatio));
        DrawCircleV(position, radius, drawColor);
    }
}

void Game::DrawActiveEffects() const
{
    bool hasAny = false;
    for (const std::unique_ptr<game::PowerUpEffect>& effect : powerUpSystem.activeEffects())
    {
        if (effect && !effect->isFinished())
        {
            hasAny = true;
            break;
        }
    }

    if (!hasAny)
    {
        return;
    }

    const float panelY = std::clamp((float)screenHeight * 0.52F, HUD_HEIGHT + 24.0F, (float)screenHeight - 260.0F);
    const Rectangle panel {screenWidth - 298.0F, panelY, 278.0F, 228.0F};
    DrawRectangleRounded(panel, 0.16F, 8, ColorAlpha(BLACK, 0.30F));
    DrawRectangleRoundedLines(panel, 0.16F, 8, 1.0F, ColorAlpha(WHITE, 0.25F));
    DrawLocalized(u8"\u5f53\u524d\u9053\u5177\u6548\u679c", "Active Effects", panel.x + 12.0F, panel.y + 8.0F, 20.0F, ColorAlpha(WHITE, 0.92F));

    int timerRow = 0;
    int permanentRow = 0;
    for (const std::unique_ptr<game::PowerUpEffect>& effect : powerUpSystem.activeEffects())
    {
        if (!effect || effect->isFinished())
        {
            continue;
        }

        const Color tint = game_style::powerUpColor(effect->type());
        const char* labelZh = game_style::effectLabelZh(effect->type());
        const char* labelEn = game_style::effectLabelEn(effect->type());

        if (effect->isPermanent())
        {
            const Rectangle chip {
                panel.x + 12.0F,
                panel.y + 176.0F + (float)permanentRow * 24.0F,
                248.0F,
                20.0F,
            };
            DrawRectangleRounded(chip, 0.30F, 6, ColorAlpha(tint, 0.82F));
            DrawLocalized(labelZh, labelEn, chip.x + 8.0F, chip.y + 2.0F, 15.0F, BLACK);
            DrawLocalized(u8"\u5e38\u9a7b", "PERM", chip.x + 194.0F, chip.y + 2.0F, 14.0F, BLACK);
            ++permanentRow;
            continue;
        }

        const float duration = std::max(effect->durationSeconds(), 0.001F);
        const float ratio = std::clamp(effect->remainingSeconds() / duration, 0.0F, 1.0F);
        const float y = panel.y + 38.0F + (float)timerRow * 34.0F;

        DrawLocalized(labelZh, labelEn, panel.x + 12.0F, y, 16.0F, ColorAlpha(WHITE, 0.9F));

        const Rectangle barBg {panel.x + 12.0F, y + 16.0F, 248.0F, 12.0F};
        DrawRectangleRounded(barBg, 0.32F, 8, ColorAlpha(BLACK, 0.55F));

        Rectangle barFill = barBg;
        barFill.width *= ratio;
        DrawRectangleRounded(barFill, 0.32F, 8, ColorAlpha(tint, 0.95F));
        DrawRectangleRoundedLines(barBg, 0.32F, 8, 1.0F, ColorAlpha(WHITE, 0.22F));

        char timerBuffer[24] {};
        std::snprintf(timerBuffer, sizeof(timerBuffer), "%.1fs", effect->remainingSeconds());
        DrawText(timerBuffer, (int)panel.x + 188, (int)y - 1, 16, ColorAlpha(WHITE, 0.95F));
        ++timerRow;
    }
}

void Game::Draw()
{
    BeginDrawing();

    const float timeSeconds = (float)GetTime();
    const Color topGradient = isDarkMode ? Color {20, 29, 47, 255} : Color {212, 233, 252, 255};
    const Color bottomGradient = isDarkMode ? Color {6, 10, 20, 255} : Color {244, 247, 255, 255};
    ClearBackground(bottomGradient);
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, topGradient, bottomGradient);

    const Texture2D* backgroundTexture = ResolveCurrentBackground();
    if (backgroundTexture != nullptr)
    {
        DrawTextureCover(*backgroundTexture, screenWidth, screenHeight, isDarkMode ? 0.42F : 0.34F);
        const Color veilTop = isDarkMode ? Color {8, 14, 28, 166} : Color {236, 244, 255, 156};
        const Color veilBottom = isDarkMode ? Color {3, 6, 14, 184} : Color {248, 250, 255, 170};
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, veilTop, veilBottom);
    }

    for (int orb = 0; orb < 6; ++orb)
    {
        const float phase = timeSeconds * 0.22F + (float)orb * 0.87F;
        const float x = (float)screenWidth * (0.11F + 0.14F * (float)orb) + std::sin(phase) * 24.0F;
        const float y = (float)screenHeight * (0.13F + 0.12F * (float)(orb % 4)) + std::cos(phase * 1.23F) * 20.0F;
        const float radius = 66.0F + (float)orb * 13.0F;
        const Color orbColor = isDarkMode ? Color {70, 110, 190, 24} : Color {100, 145, 226, 42};
        DrawCircleV(Vector2 {x, y}, radius, orbColor);
    }

    const Color wallColor = isDarkMode ? Color {63, 82, 120, 210} : Color {117, 151, 194, 170};
    const Color textColor = isDarkMode ? Color {225, 236, 255, 255} : Color {22, 33, 54, 255};

    DrawRectangle(0, 0, (int)WALL_THICKNESS, screenHeight, wallColor);
    DrawRectangle(screenWidth - (int)WALL_THICKNESS, 0, (int)WALL_THICKNESS, screenHeight, wallColor);
    DrawRectangle(0, 0, screenWidth, (int)WALL_THICKNESS, wallColor);

    const Rectangle hudPanel {16.0F, 12.0F, (float)screenWidth - 32.0F, HUD_HEIGHT};
    DrawRectangleRounded(hudPanel, 0.18F, 10, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.42F : 0.74F));
    DrawRectangleRoundedLines(hudPanel, 0.18F, 10, 1.5F, ColorAlpha(WHITE, 0.22F));

    DrawLocalizedf(u8"\u5206\u6570", "SCORE", score, 30.0F, 24.0F, 24.0F, textColor);
    DrawLocalizedf(
        u8"\u751f\u547d",
        "LIVES",
        debugMode ? 999 : lives,
        30.0F,
        50.0F,
        18.0F,
        debugMode ? GREEN : Color {255, 132, 132, 255});
    DrawLocalizedf(
        u8"\u7403\u6570",
        "BALLS",
        (int)ballManager.balls().size(),
        150.0F,
        50.0F,
        18.0F,
        Color {114, 247, 194, 255});

    if (hasUiFont)
    {
        char stageText[64] {};
        std::snprintf(stageText, sizeof(stageText), u8"\u5173\u5361 %d / %d", currentLevel, (int)game_style::levelConfigs().size());
        DrawTextEx(uiFont, stageText, Vector2 {(float)screenWidth / 2.0F - 84.0F, 36.0F}, 22.0F, 0.0F, textColor);
    }
    else
    {
        DrawText(
            TextFormat("STAGE %d / %d", currentLevel, (int)game_style::levelConfigs().size()),
            screenWidth / 2 - 78,
            36,
            22,
            textColor);
    }

    DrawLocalized(
        u8"\u672c\u5173\u6e05\u7a7a\u81ea\u52a8\u8fdb\u5165\u4e0b\u4e00\u5173",
        "AUTO ADVANCE ON CLEAR",
        (float)screenWidth / 2.0F - 136.0F,
        58.0F,
        14.0F,
        ColorAlpha(textColor, 0.72F));

    if (debugMode)
    {
        DrawLocalized(u8"[\u8c03\u8bd5: \u65e0\u654c]", "[DEBUG: INVINCIBLE]", (float)screenWidth / 2.0F - 112.0F, 14.0F, 16.0F, GREEN);
    }

    const Rectangle btnRec = game_style::pauseButtonRect(screenWidth, PAUSE_BUTTON_SIZE);
    DrawRectangleRounded(btnRec, 0.25F, 4, ColorAlpha(isDarkMode ? WHITE : BLACK, 0.16F));
    DrawRectangleRoundedLines(btnRec, 0.25F, 4, 1.8F, ColorAlpha(WHITE, 0.35F));
    if (currentState == GameState::PLAYING)
    {
        DrawRectangle((int)btnRec.x + 10, (int)btnRec.y + 7, 4, 20, textColor);
        DrawRectangle((int)btnRec.x + 20, (int)btnRec.y + 7, 4, 20, textColor);
    }
    else
    {
        const Vector2 p1 {btnRec.x + 11.0F, btnRec.y + 7.0F};
        const Vector2 p2 {btnRec.x + 11.0F, btnRec.y + 27.0F};
        const Vector2 p3 {btnRec.x + 26.0F, btnRec.y + 17.0F};
        DrawTriangle(p1, p2, p3, textColor);
    }

    if (currentState == GameState::PLAYING || currentState == GameState::PAUSED || currentState == GameState::GAMEOVER
        || victory)
    {
        DrawEdgeParticles();
        DrawParticles();

        for (const game::Ball& managedBall : ballManager.balls())
        {
            if (!managedBall.active)
            {
                continue;
            }

            DrawBallTrail(managedBall, ballColor, timeSeconds);
            DrawCircleV(game_style::toRayVec(managedBall.position), managedBall.radius + 2.2F, ColorAlpha(ballColor, 0.22F));
            DrawCircleV(game_style::toRayVec(managedBall.position), managedBall.radius, ballColor);
        }

        paddle.Draw(paddleColor);

        float brickGlow = 0.0F;
        const Color activeBrickColor = lerpSequence(brickColor, powerUpSystem.activeEffects(), timeSeconds, &brickGlow);
        for (std::size_t brickIndex = 0; brickIndex < bricks.size(); ++brickIndex)
        {
            Brick& brick = bricks[brickIndex];
            const Color drawColor = GetBrickDisplayColor(brickIndex, activeBrickColor, timeSeconds, brickGlow);
            brick.Draw(drawColor);
            if (brick.IsActive() && brickGlow > 0.0F)
            {
                const Rectangle rect = brick.GetRect();
                DrawRectangleRoundedLines(
                    rect,
                    0.2F,
                    4,
                    2.0F,
                    ColorAlpha(drawColor, std::clamp(0.18F + brickGlow * 0.45F, 0.18F, 0.68F)));
            }
            if (brick.IsActive() && brickIndex < brickPowerUps.size() && brickPowerUps[brickIndex].has_value())
            {
                const Rectangle rect = brick.GetRect();
                DrawRectangleRoundedLines(rect, 0.2F, 4, 3.0F, ColorAlpha(game_style::powerUpColor(*brickPowerUps[brickIndex]), 0.88F));
            }
        }

        DrawPowerUps();
        DrawActiveEffects();
    }

    if (currentState == GameState::MENU)
    {
        const Rectangle hero {screenWidth / 2.0F - 350.0F, screenHeight / 2.0F - 220.0F, 700.0F, 500.0F};
        DrawRectangleRounded(hero, 0.08F, 10, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.52F : 0.74F));
        DrawRectangleRoundedLines(hero, 0.08F, 10, 2.0F, ColorAlpha(WHITE, 0.28F));

        DrawLocalized(u8"\u6253\u7816\u5757", "BREAKOUT", (float)screenWidth / 2.0F - 150.0F, (float)screenHeight / 2.0F - 175.0F, 66.0F, ColorAlpha(WHITE, 0.98F));
        DrawLocalized(u8"\u7ade\u6280\u7248", "ELITE ARENA", (float)screenWidth / 2.0F - 82.0F, (float)screenHeight / 2.0F - 112.0F, 32.0F, ColorAlpha(SKYBLUE, 0.92F));
        DrawLocalized(
            u8"\u6309 ENTER \u4ece\u7b2c\u4e00\u5173\u5f00\u59cb",
            "Press ENTER to start run from Stage 1",
            (float)screenWidth / 2.0F - 182.0F,
            (float)screenHeight / 2.0F - 42.0F,
            24.0F,
            ColorAlpha(textColor, 0.86F));
        DrawLocalized(
            u8"\u5173\u5361\u4f1a\u81ea\u52a8\u8854\u63a5\uff0c\u9053\u5177\u548c\u591a\u7403\u4f1a\u7ee7\u7eed\u751f\u6548",
            "Stages auto-chain. Effects and multiball stay in the action.",
            (float)screenWidth / 2.0F - 214.0F,
            (float)screenHeight / 2.0F - 6.0F,
            20.0F,
            ColorAlpha(textColor, 0.72F));

        const Rectangle themeBtn {screenWidth / 2.0F - 250.0F, screenHeight / 2.0F + 90.0F, 230.0F, 54.0F};
        const Rectangle ballBtn {screenWidth / 2.0F + 20.0F, screenHeight / 2.0F + 90.0F, 230.0F, 54.0F};
        const Rectangle paddleBtn {screenWidth / 2.0F - 250.0F, screenHeight / 2.0F + 162.0F, 230.0F, 54.0F};
        const Rectangle brickBtn {screenWidth / 2.0F + 20.0F, screenHeight / 2.0F + 162.0F, 230.0F, 54.0F};

        DrawRectangleRounded(themeBtn, 0.22F, 6, isDarkMode ? Color {170, 176, 188, 220} : Color {86, 102, 132, 220});
        DrawLocalized(
            isDarkMode ? u8"\u4e3b\u9898  \u6697\u8272" : u8"\u4e3b\u9898  \u4eae\u8272",
            isDarkMode ? "Theme  DARK" : "Theme  LIGHT",
            themeBtn.x + 26.0F,
            themeBtn.y + 14.0F,
            22.0F,
            WHITE);

        DrawRectangleRounded(ballBtn, 0.22F, 6, ballColor);
        DrawLocalized(u8"\u7403\u4f53\u914d\u8272", "Ball Tone", ballBtn.x + 58.0F, ballBtn.y + 14.0F, 22.0F, WHITE);

        DrawRectangleRounded(paddleBtn, 0.22F, 6, paddleColor);
        DrawLocalized(u8"\u6321\u677f\u914d\u8272", "Paddle Tone", paddleBtn.x + 43.0F, paddleBtn.y + 14.0F, 22.0F, WHITE);

        DrawRectangleRounded(brickBtn, 0.22F, 6, brickColor);
        DrawLocalized(u8"\u7816\u5757\u914d\u8272", "Brick Tone", brickBtn.x + 50.0F, brickBtn.y + 14.0F, 22.0F, WHITE);
    }
    else if (currentState == GameState::PAUSED)
    {
        const Rectangle panel {screenWidth / 2.0F - 210.0F, screenHeight / 2.0F - 80.0F, 420.0F, 170.0F};
        DrawRectangleRounded(panel, 0.12F, 10, ColorAlpha(BLACK, 0.62F));
        DrawRectangleRoundedLines(panel, 0.12F, 10, 2.0F, ColorAlpha(WHITE, 0.22F));
        DrawLocalized(u8"\u5df2\u6682\u505c", "PAUSED", (float)screenWidth / 2.0F - 82.0F, (float)screenHeight / 2.0F - 22.0F, 48.0F, ColorAlpha(ORANGE, 0.95F));
        DrawLocalized(
            u8"\u6309 P \u6216\u70b9\u51fb\u53f3\u4e0a\u89d2\u7ee7\u7eed",
            "Press P or click top-right button",
            (float)screenWidth / 2.0F - 158.0F,
            (float)screenHeight / 2.0F + 30.0F,
            20.0F,
            ColorAlpha(WHITE, 0.88F));
    }
    else if (currentState == GameState::GAMEOVER)
    {
        const Rectangle panel {screenWidth / 2.0F - 230.0F, screenHeight / 2.0F - 96.0F, 460.0F, 190.0F};
        DrawRectangleRounded(panel, 0.10F, 10, ColorAlpha(BLACK, 0.66F));
        DrawRectangleRoundedLines(panel, 0.10F, 10, 2.0F, ColorAlpha(WHITE, 0.20F));
        DrawLocalized(u8"\u6e38\u620f\u7ed3\u675f", "GAME OVER", (float)screenWidth / 2.0F - 145.0F, (float)screenHeight / 2.0F - 20.0F, 54.0F, ColorAlpha(RED, 0.95F));
        DrawLocalizedf(
            u8"\u6700\u7ec8\u5206\u6570",
            "Final Score",
            score,
            (float)screenWidth / 2.0F - 86.0F,
            (float)screenHeight / 2.0F + 42.0F,
            24.0F,
            ColorAlpha(WHITE, 0.90F));
        DrawLocalized(u8"\u6309 Q \u9000\u51fa", "Press Q to quit", (float)screenWidth / 2.0F - 74.0F, (float)screenHeight / 2.0F + 70.0F, 18.0F, ColorAlpha(WHITE, 0.72F));
    }
    else if (victory)
    {
        const Rectangle panel {screenWidth / 2.0F - 250.0F, screenHeight / 2.0F - 98.0F, 500.0F, 200.0F};
        DrawRectangleRounded(panel, 0.10F, 10, ColorAlpha(BLACK, 0.65F));
        DrawRectangleRoundedLines(panel, 0.10F, 10, 2.0F, ColorAlpha(WHITE, 0.26F));
        DrawLocalized(
            u8"\u5168\u90e8\u5173\u5361\u901a\u5173",
            "ALL STAGES CLEARED",
            (float)screenWidth / 2.0F - 216.0F,
            (float)screenHeight / 2.0F - 22.0F,
            48.0F,
            ColorAlpha(GREEN, 0.96F));
        DrawLocalizedf(
            u8"\u6700\u7ec8\u5206\u6570",
            "FINAL SCORE",
            score,
            (float)screenWidth / 2.0F - 124.0F,
            (float)screenHeight / 2.0F + 44.0F,
            28.0F,
            ColorAlpha(WHITE, 0.94F));
        DrawLocalized(
            u8"\u6309 Q \u7ed3\u675f\u672c\u5c40",
            "Press Q to quit run",
            (float)screenWidth / 2.0F - 86.0F,
            (float)screenHeight / 2.0F + 78.0F,
            18.0F,
            ColorAlpha(WHITE, 0.74F));
    }

    EndDrawing();
}
