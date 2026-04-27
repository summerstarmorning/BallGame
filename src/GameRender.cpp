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

void DrawGlassPanel(Rectangle panel, Color fill, Color border, Color glow, float roundness = 0.20F)
{
    DrawRectangleRounded(panel, roundness, 10, fill);

    Rectangle glowRect {panel.x - 2.0F, panel.y - 2.0F, panel.width + 4.0F, panel.height + 4.0F};
    DrawRectangleRounded(glowRect, roundness, 10, ColorAlpha(glow, 0.12F));

    Rectangle sheen {
        panel.x + 2.0F,
        panel.y + 2.0F,
        std::max(0.0F, panel.width - 4.0F),
        std::max(18.0F, panel.height * 0.42F),
    };
    DrawRectangleRounded(sheen, roundness, 10, ColorAlpha(WHITE, 0.035F));
    DrawRectangleRoundedLines(panel, roundness, 10, 1.4F, border);
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

void Game::DrawDisplayText(const char* text, Vector2 position, float fontSize, float spacing, Color color) const
{
    if (hasDisplayFont)
    {
        DrawTextEx(displayFont, text, position, fontSize, spacing, color);
        return;
    }

    DrawText(text, (int)position.x, (int)position.y, (int)fontSize, color);
}

float Game::MeasureDisplayTextWidth(const char* text, float fontSize, float spacing) const
{
    if (hasDisplayFont)
    {
        return MeasureTextEx(displayFont, text, fontSize, spacing).x;
    }

    return (float)MeasureText(text, (int)fontSize);
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
    bool hasAny = pendingPierceCharges > 0;
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

    const Color panelFill = isDarkMode ? Color {10, 24, 40, 186} : Color {255, 255, 255, 210};
    const Color panelBorder = isDarkMode ? Color {124, 212, 255, 82} : Color {60, 116, 166, 90};
    const Color panelGlow = isDarkMode ? Color {72, 196, 255, 76} : Color {121, 182, 235, 70};
    const Color textColor = isDarkMode ? Color {232, 241, 255, 255} : Color {23, 34, 55, 255};
    const Color accent = isDarkMode ? Color {104, 227, 255, 255} : Color {37, 117, 181, 255};

    const float panelWidth = std::clamp((float)screenWidth * 0.255F, 286.0F, 332.0F);
    const float panelHeight = 236.0F;
    const float panelY = std::clamp((float)screenHeight * 0.49F, HUD_HEIGHT + 26.0F, (float)screenHeight - panelHeight - 22.0F);
    const Rectangle panel {(float)screenWidth - panelWidth - 20.0F, panelY, panelWidth, panelHeight};
    DrawGlassPanel(panel, panelFill, panelBorder, panelGlow, 0.18F);

    DrawDisplayText("LIVE MODIFIERS", Vector2 {panel.x + 16.0F, panel.y + 14.0F}, 19.0F, 1.2F, accent);
    DrawLocalized(
        u8"\u5f53\u524d\u6218\u6597\u589e\u76ca",
        "Combat stack online",
        panel.x + 16.0F,
        panel.y + 38.0F,
        18.0F,
        ColorAlpha(textColor, 0.88F));

    float cursorY = panel.y + 72.0F;
    if (pendingPierceCharges > 0)
    {
        const Rectangle pierceChip {panel.x + 14.0F, cursorY, panel.width - 28.0F, 28.0F};
        const Color tint = game_style::powerUpColor(game::PowerUpType::PierceBall);
        DrawRectangleRounded(pierceChip, 0.28F, 8, ColorAlpha(tint, 0.88F));
        DrawDisplayText("PIERCE SHOT", Vector2 {pierceChip.x + 12.0F, pierceChip.y + 5.0F}, 15.0F, 1.0F, BLACK);
        char chargesBuffer[24] {};
        std::snprintf(chargesBuffer, sizeof(chargesBuffer), "x%d", pendingPierceCharges);
        DrawDisplayText(
            chargesBuffer,
            Vector2 {pierceChip.x + pierceChip.width - 36.0F, pierceChip.y + 5.0F},
            15.0F,
            1.0F,
            BLACK);
        cursorY += 36.0F;
    }

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
                panel.x + 14.0F,
                panel.y + panel.height - 30.0F - (float)permanentRow * 28.0F,
                panel.width - 28.0F,
                22.0F,
            };
            DrawRectangleRounded(chip, 0.28F, 8, ColorAlpha(tint, 0.86F));
            DrawLocalized(labelZh, labelEn, chip.x + 8.0F, chip.y + 2.0F, 15.0F, BLACK);
            DrawDisplayText("PERM", Vector2 {chip.x + chip.width - 46.0F, chip.y + 4.0F}, 12.0F, 1.2F, BLACK);
            ++permanentRow;
            continue;
        }

        const float duration = std::max(effect->durationSeconds(), 0.001F);
        const float ratio = std::clamp(effect->remainingSeconds() / duration, 0.0F, 1.0F);
        DrawLocalized(labelZh, labelEn, panel.x + 16.0F, cursorY - 2.0F, 16.0F, ColorAlpha(textColor, 0.92F));

        char timerBuffer[24] {};
        std::snprintf(timerBuffer, sizeof(timerBuffer), "%.1fs", effect->remainingSeconds());
        const float timerWidth = MeasureDisplayTextWidth(timerBuffer, 14.0F, 1.0F);
        DrawDisplayText(
            timerBuffer,
            Vector2 {panel.x + panel.width - 16.0F - timerWidth, cursorY - 1.0F},
            14.0F,
            1.0F,
            ColorAlpha(textColor, 0.94F));

        const Rectangle barBg {panel.x + 16.0F, cursorY + 18.0F, panel.width - 32.0F, 12.0F};
        DrawRectangleRounded(barBg, 0.32F, 8, ColorAlpha(BLACK, 0.45F));
        if (ratio > 0.001F)
        {
            Rectangle barFill = barBg;
            barFill.width *= ratio;
            DrawRectangleRounded(barFill, 0.32F, 8, ColorAlpha(tint, 0.96F));
        }
        DrawRectangleRoundedLines(barBg, 0.32F, 8, 1.0F, ColorAlpha(WHITE, 0.18F));
        cursorY += 40.0F;
    }
}

void Game::Draw()
{
    BeginDrawing();

    const float timeSeconds = (float)GetTime();
    const Color topGradient = isDarkMode ? Color {10, 18, 34, 255} : Color {207, 224, 243, 255};
    const Color bottomGradient = isDarkMode ? Color {3, 8, 18, 255} : Color {245, 247, 252, 255};
    const Color accent = isDarkMode ? Color {93, 222, 255, 255} : Color {33, 112, 173, 255};
    const Color accentWarm = isDarkMode ? Color {255, 190, 99, 255} : Color {226, 145, 69, 255};
    const Color accentSuccess = isDarkMode ? Color {104, 240, 179, 255} : Color {36, 146, 102, 255};
    const Color textColor = isDarkMode ? Color {230, 239, 255, 255} : Color {21, 33, 55, 255};
    const Color subTextColor = ColorAlpha(textColor, isDarkMode ? 0.74F : 0.68F);
    const Color panelFill = isDarkMode ? Color {10, 24, 40, 182} : Color {255, 255, 255, 208};
    const Color panelBorder = isDarkMode ? Color {132, 214, 255, 76} : Color {71, 118, 169, 84};
    const Color panelGlow = isDarkMode ? Color {72, 194, 255, 72} : Color {112, 177, 234, 68};

    ClearBackground(bottomGradient);
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, topGradient, bottomGradient);

    const Texture2D* backgroundTexture = ResolveCurrentBackground();
    if (backgroundTexture != nullptr)
    {
        DrawTextureCover(*backgroundTexture, screenWidth, screenHeight, isDarkMode ? 0.38F : 0.26F);
        const Color veilTop = isDarkMode ? Color {7, 12, 24, 174} : Color {241, 246, 255, 128};
        const Color veilBottom = isDarkMode ? Color {2, 4, 11, 198} : Color {250, 251, 255, 160};
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, veilTop, veilBottom);
    }

    for (int orb = 0; orb < 8; ++orb)
    {
        const float phase = timeSeconds * (0.17F + 0.02F * (float)(orb % 3)) + (float)orb * 0.86F;
        const float x = (float)screenWidth * (0.08F + 0.11F * (float)(orb % 5)) + std::sin(phase) * 30.0F;
        const float y = (float)screenHeight * (0.10F + 0.14F * (float)(orb / 2)) + std::cos(phase * 1.18F) * 26.0F;
        const float radius = 72.0F + (float)(orb % 4) * 22.0F;
        const Color orbColor = (orb % 2 == 0)
            ? (isDarkMode ? Color {78, 180, 255, 26} : Color {98, 162, 231, 30})
            : (isDarkMode ? Color {255, 170, 92, 18} : Color {244, 184, 96, 26});
        DrawCircleV(Vector2 {x, y}, radius, orbColor);
    }

    for (int y = 0; y < screenHeight; y += 4)
    {
        DrawLine(0, y, screenWidth, y, ColorAlpha(isDarkMode ? SKYBLUE : BLUE, isDarkMode ? 0.025F : 0.015F));
    }

    const Color wallColor = isDarkMode ? Color {60, 93, 139, 208} : Color {108, 142, 190, 180};
    DrawRectangle(0, 0, (int)WALL_THICKNESS, screenHeight, wallColor);
    DrawRectangle(screenWidth - (int)WALL_THICKNESS, 0, (int)WALL_THICKNESS, screenHeight, wallColor);
    DrawRectangle(0, 0, screenWidth, (int)WALL_THICKNESS, wallColor);
    DrawRectangleGradientV(0, 0, screenWidth, 18, ColorAlpha(accent, 0.18F), BLANK);

    const float hudGap = 12.0F;
    const float leftWidth = std::clamp((float)screenWidth * 0.26F, 252.0F, 304.0F);
    const float rightWidth = std::clamp((float)screenWidth * 0.24F, 244.0F, 290.0F);
    const float centerWidth = std::max(240.0F, (float)screenWidth - 36.0F - leftWidth - rightWidth - hudGap * 2.0F);

    const Rectangle hudLeft {18.0F, 12.0F, leftWidth, HUD_HEIGHT};
    const Rectangle hudCenter {hudLeft.x + hudLeft.width + hudGap, 12.0F, centerWidth, HUD_HEIGHT};
    const Rectangle hudRight {hudCenter.x + hudCenter.width + hudGap, 12.0F, rightWidth, HUD_HEIGHT};

    DrawGlassPanel(hudLeft, panelFill, panelBorder, panelGlow, 0.20F);
    DrawGlassPanel(hudCenter, panelFill, panelBorder, panelGlow, 0.20F);
    DrawGlassPanel(hudRight, panelFill, panelBorder, panelGlow, 0.20F);

    char scoreBuffer[32] {};
    std::snprintf(scoreBuffer, sizeof(scoreBuffer), "%d", score);
    DrawDisplayText("SCORE", Vector2 {hudLeft.x + 16.0F, hudLeft.y + 10.0F}, 13.0F, 1.6F, accent);
    DrawDisplayText(scoreBuffer, Vector2 {hudLeft.x + 16.0F, hudLeft.y + 28.0F}, 27.0F, 1.2F, ColorAlpha(textColor, 0.98F));

    const float chipY = hudLeft.y + 16.0F;
    const float chipX = hudLeft.x + hudLeft.width - 136.0F;
    const Color chipFill = ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.26F : 0.42F);
    const Rectangle livesChip {chipX, chipY, 38.0F, 46.0F};
    const Rectangle ballsChip {chipX + 44.0F, chipY, 38.0F, 46.0F};
    const Rectangle pierceChip {chipX + 88.0F, chipY, 38.0F, 46.0F};
    DrawGlassPanel(livesChip, chipFill, ColorAlpha(panelBorder, 0.8F), ColorAlpha(panelGlow, 0.8F), 0.24F);
    DrawGlassPanel(ballsChip, chipFill, ColorAlpha(panelBorder, 0.8F), ColorAlpha(panelGlow, 0.8F), 0.24F);
    DrawGlassPanel(pierceChip, chipFill, ColorAlpha(panelBorder, 0.8F), ColorAlpha(panelGlow, 0.8F), 0.24F);
    DrawDisplayText("L", Vector2 {livesChip.x + 12.0F, livesChip.y + 6.0F}, 12.0F, 1.0F, accentWarm);
    DrawDisplayText("B", Vector2 {ballsChip.x + 11.0F, ballsChip.y + 6.0F}, 12.0F, 1.0F, accentSuccess);
    DrawDisplayText("P", Vector2 {pierceChip.x + 11.0F, pierceChip.y + 6.0F}, 12.0F, 1.0F, game_style::powerUpColor(game::PowerUpType::PierceBall));

    char smallMetric[16] {};
    std::snprintf(smallMetric, sizeof(smallMetric), "%d", debugMode ? 999 : lives);
    DrawDisplayText(smallMetric, Vector2 {livesChip.x + 11.0F, livesChip.y + 20.0F}, 18.0F, 1.0F, ColorAlpha(textColor, 0.98F));
    std::snprintf(smallMetric, sizeof(smallMetric), "%d", (int)ballManager.balls().size());
    DrawDisplayText(smallMetric, Vector2 {ballsChip.x + 11.0F, ballsChip.y + 20.0F}, 18.0F, 1.0F, ColorAlpha(textColor, 0.98F));
    std::snprintf(smallMetric, sizeof(smallMetric), "%d", pendingPierceCharges);
    DrawDisplayText(smallMetric, Vector2 {pierceChip.x + 11.0F, pierceChip.y + 20.0F}, 18.0F, 1.0F, ColorAlpha(textColor, 0.98F));

    DrawDisplayText("BREAKOUT", Vector2 {hudCenter.x + 18.0F, hudCenter.y + 10.0F}, 22.0F, 2.0F, ColorAlpha(textColor, 0.98F));
    DrawDisplayText("ASCEND", Vector2 {hudCenter.x + 20.0F, hudCenter.y + 40.0F}, 16.0F, 1.8F, accent);
    char stageBuffer[48] {};
    std::snprintf(stageBuffer, sizeof(stageBuffer), "STAGE %d / %d", currentLevel, (int)game_style::levelConfigs().size());
    const float stageWidth = MeasureDisplayTextWidth(stageBuffer, 16.0F, 1.3F);
    DrawDisplayText(
        stageBuffer,
        Vector2 {hudCenter.x + hudCenter.width - stageWidth - 18.0F, hudCenter.y + 12.0F},
        16.0F,
        1.3F,
        accentWarm);
    DrawLocalized(
        u8"\u81ea\u52a8\u8854\u63a5\u8fde\u7eed\u95ef\u5173",
        "Auto-chain campaign run",
        hudCenter.x + 18.0F,
        hudCenter.y + 58.0F,
        15.0F,
        subTextColor);
    if (debugMode)
    {
        const Rectangle debugChip {hudCenter.x + hudCenter.width - 116.0F, hudCenter.y + 42.0F, 96.0F, 24.0F};
        DrawRectangleRounded(debugChip, 0.40F, 8, ColorAlpha(GREEN, 0.82F));
        DrawDisplayText("DEBUG", Vector2 {debugChip.x + 16.0F, debugChip.y + 5.0F}, 12.0F, 1.2F, BLACK);
    }

    DrawDisplayText("COINS", Vector2 {hudRight.x + 16.0F, hudRight.y + 10.0F}, 13.0F, 1.4F, accentWarm);
    DrawDisplayText("BEST", Vector2 {hudRight.x + 94.0F, hudRight.y + 10.0F}, 13.0F, 1.4F, accent);
    DrawDisplayText("RUNS", Vector2 {hudRight.x + 170.0F, hudRight.y + 10.0F}, 13.0F, 1.4F, accentSuccess);

    char rightMetric[24] {};
    std::snprintf(rightMetric, sizeof(rightMetric), "%d", playerProfile.softCurrency);
    DrawDisplayText(rightMetric, Vector2 {hudRight.x + 16.0F, hudRight.y + 30.0F}, 24.0F, 1.0F, ColorAlpha(textColor, 0.98F));
    std::snprintf(rightMetric, sizeof(rightMetric), "%d", std::max(playerProfile.bestScore, score));
    DrawDisplayText(rightMetric, Vector2 {hudRight.x + 94.0F, hudRight.y + 30.0F}, 24.0F, 1.0F, ColorAlpha(textColor, 0.98F));
    std::snprintf(rightMetric, sizeof(rightMetric), "%d", std::max(playerProfile.totalRuns, 1));
    DrawDisplayText(rightMetric, Vector2 {hudRight.x + 170.0F, hudRight.y + 30.0F}, 24.0F, 1.0F, ColorAlpha(textColor, 0.98F));
    DrawLocalized(
        u8"\u8f6f\u8d27\u5e01 \u5386\u53f2\u6700\u9ad8 \u603b\u5c40\u6570",
        "Meta profile snapshot",
        hudRight.x + 16.0F,
        hudRight.y + 58.0F,
        14.0F,
        subTextColor);

    const Rectangle btnRec = game_style::pauseButtonRect(screenWidth, PAUSE_BUTTON_SIZE);
    DrawGlassPanel(btnRec, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.24F : 0.44F), panelBorder, panelGlow, 0.26F);
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

        const Color renderedBallColor = pendingPierceCharges > 0
            ? game_style::mixColor(ballColor, game_style::powerUpColor(game::PowerUpType::PierceBall), 0.44F)
            : ballColor;

        for (const game::Ball& managedBall : ballManager.balls())
        {
            if (!managedBall.active)
            {
                continue;
            }

            DrawBallTrail(managedBall, renderedBallColor, timeSeconds);
            DrawCircleV(
                game_style::toRayVec(managedBall.position),
                managedBall.radius + 2.2F,
                ColorAlpha(renderedBallColor, pendingPierceCharges > 0 ? 0.34F : 0.22F));
            if (pendingPierceCharges > 0)
            {
                DrawCircleV(
                    game_style::toRayVec(managedBall.position),
                    managedBall.radius + 5.4F,
                    ColorAlpha(renderedBallColor, 0.11F));
            }
            DrawCircleV(game_style::toRayVec(managedBall.position), managedBall.radius, renderedBallColor);
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
        const Rectangle shell {60.0F, (float)screenHeight * 0.16F, (float)screenWidth - 120.0F, (float)screenHeight * 0.64F};
        const Rectangle leftColumn {shell.x + 18.0F, shell.y + 18.0F, shell.width * 0.53F, shell.height - 36.0F};
        const Rectangle rightColumn {
            leftColumn.x + leftColumn.width + 18.0F,
            shell.y + 18.0F,
            shell.width - leftColumn.width - 36.0F,
            shell.height - 36.0F,
        };
        const MenuStyleButtonRects styleButtons = MenuStyleButtons();
        DrawGlassPanel(shell, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.24F : 0.18F), panelBorder, panelGlow, 0.10F);
        DrawGlassPanel(leftColumn, panelFill, panelBorder, panelGlow, 0.10F);
        DrawGlassPanel(rightColumn, panelFill, panelBorder, panelGlow, 0.10F);

        DrawLocalized(
            u8"\u8857\u673a\u7834\u9635",
            "BREAKOUT",
            leftColumn.x + 24.0F,
            leftColumn.y + 26.0F,
            58.0F,
            ColorAlpha(textColor, 0.98F));
        DrawDisplayText("BREAKOUT", Vector2 {leftColumn.x + 28.0F, leftColumn.y + 96.0F}, 22.0F, 2.2F, accent);
        DrawLocalized(
            u8"\u5546\u4e1a\u5316\u8857\u673a\u539f\u578b",
            "Commercial-ready arcade front end",
            leftColumn.x + 28.0F,
            leftColumn.y + 130.0F,
            24.0F,
            ColorAlpha(textColor, 0.90F));
        DrawLocalized(
            u8"\u9ad8\u4fdd\u771f\u78b0\u649e\u3001\u8fde\u7eed\u95ef\u5173\u548c\u957f\u671f\u6210\u957f\uff0c\u5168\u90e8\u6536\u675f\u5230\u540c\u4e00\u5c40\u4f53\u9a8c",
            "High-fidelity collisions, chained stages, and retention inside one run",
            leftColumn.x + 28.0F,
            leftColumn.y + 164.0F,
            20.0F,
            ColorAlpha(textColor, 0.90F));

        const Rectangle enterChip {leftColumn.x + 28.0F, leftColumn.y + 212.0F, 174.0F, 34.0F};
        DrawRectangleRounded(enterChip, 0.45F, 8, ColorAlpha(accentWarm, 0.20F));
        DrawRectangleRoundedLines(enterChip, 0.45F, 8, 1.3F, ColorAlpha(accentWarm, 0.56F));
        DrawDisplayText("PRESS ENTER", Vector2 {enterChip.x + 16.0F, enterChip.y + 9.0F}, 14.0F, 1.5F, accentWarm);
        DrawLocalized(
            u8"\u6309 Enter \u7acb\u5373\u5f00\u5c40\uff0c\u7ee7\u7eed\u4f60\u7684\u4e0b\u4e00\u8f6e\u63a8\u8fdb",
            "Press Enter to continue your next run",
            leftColumn.x + 28.0F,
            leftColumn.y + 258.0F,
            22.0F,
            ColorAlpha(textColor, 0.94F));
        DrawLocalized(
            u8"\u6309 L \u6f14\u793a\u5f02\u6b65\u52a0\u8f7d\uff0c\u5f55\u5c4f\u65f6\u53ef\u76f4\u63a5\u5c55\u793a\u4e0d\u5361\u987f\u7684 Loading \u52a8\u753b",
            "Press L to demo async loading",
            leftColumn.x + 28.0F,
            leftColumn.y + 286.0F,
            16.0F,
            ColorAlpha(accentWarm, 0.96F));

        const float cardY = leftColumn.y + leftColumn.height - 124.0F;
        const float cardGap = 12.0F;
        const Rectangle startBand {leftColumn.x + 26.0F, cardY - 104.0F, leftColumn.width - 52.0F, 84.0F};
        DrawRectangleRounded(startBand, 0.14F, 10, ColorAlpha(accent, 0.18F));
        DrawRectangleRoundedLines(startBand, 0.14F, 10, 1.5F, ColorAlpha(accent, 0.58F));
        DrawDisplayText("RUN BRIEF", Vector2 {startBand.x + 18.0F, startBand.y + 12.0F}, 16.0F, 1.6F, accent);
        DrawLocalized(
            u8"\u672c\u8f6e\u5165\u53e3",
            "Run entry",
            startBand.x + 18.0F,
            startBand.y + 32.0F,
            24.0F,
            ColorAlpha(textColor, 0.96F));
        DrawLocalized(
            u8"\u649e\u7a7f\u7816\u9635\u3001\u7d2f\u79ef\u8f6f\u8d27\u5e01\u3001\u6301\u7eed\u5237\u65b0\u6700\u4f73\u6210\u7ee9",
            "Break walls, earn currency, and keep pushing your best",
            startBand.x + 18.0F,
            startBand.y + 58.0F,
            16.0F,
            ColorAlpha(textColor, 0.82F));

        const float cardWidth = (leftColumn.width - 52.0F - cardGap * 2.0F) / 3.0F;
        const Rectangle bestCard {leftColumn.x + 26.0F, cardY, cardWidth, 86.0F};
        const Rectangle coinsCard {bestCard.x + bestCard.width + cardGap, cardY, cardWidth, 86.0F};
        const Rectangle stageCard {coinsCard.x + coinsCard.width + cardGap, cardY, cardWidth, 86.0F};
        DrawGlassPanel(bestCard, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.20F : 0.34F), panelBorder, panelGlow, 0.18F);
        DrawGlassPanel(coinsCard, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.20F : 0.34F), panelBorder, panelGlow, 0.18F);
        DrawGlassPanel(stageCard, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.20F : 0.34F), panelBorder, panelGlow, 0.18F);

        char menuMetric[32] {};
        DrawLocalized(u8"\u5386\u53f2\u6700\u9ad8", "BEST", bestCard.x + 12.0F, bestCard.y + 12.0F, 14.0F, accent);
        std::snprintf(menuMetric, sizeof(menuMetric), "%d", playerProfile.bestScore);
        DrawDisplayText(menuMetric, Vector2 {bestCard.x + 12.0F, bestCard.y + 38.0F}, 24.0F, 1.0F, ColorAlpha(textColor, 0.98F));

        DrawLocalized(u8"\u8f6f\u8d27\u5e01", "COINS", coinsCard.x + 12.0F, coinsCard.y + 12.0F, 14.0F, accentWarm);
        std::snprintf(menuMetric, sizeof(menuMetric), "%d", playerProfile.softCurrency);
        DrawDisplayText(menuMetric, Vector2 {coinsCard.x + 12.0F, coinsCard.y + 38.0F}, 24.0F, 1.0F, ColorAlpha(textColor, 0.98F));

        DrawLocalized(u8"\u5173\u5361", "STAGE", stageCard.x + 12.0F, stageCard.y + 12.0F, 14.0F, accentSuccess);
        std::snprintf(menuMetric, sizeof(menuMetric), "%d", std::max(playerProfile.highestStage, 1));
        DrawDisplayText(menuMetric, Vector2 {stageCard.x + 12.0F, stageCard.y + 38.0F}, 24.0F, 1.0F, ColorAlpha(textColor, 0.98F));

        const Rectangle snapshot {rightColumn.x + 18.0F, rightColumn.y + 18.0F, rightColumn.width - 36.0F, 138.0F};
        DrawGlassPanel(snapshot, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.20F : 0.34F), panelBorder, panelGlow, 0.12F);
        DrawLocalized(
            u8"\u73a9\u5bb6\u6863\u6848",
            "PLAYER SNAPSHOT",
            snapshot.x + 16.0F,
            snapshot.y + 12.0F,
            22.0F,
            accent);
        DrawLocalized(
            u8"\u672c\u5730\u6210\u957f\u3001\u5c40\u5185\u5f3a\u5ea6\u3001\u5386\u53f2\u6570\u636e\u540c\u65f6\u53ef\u89c1",
            "Long-term retention and profile economy",
            snapshot.x + 16.0F,
            snapshot.y + 46.0F,
            17.0F,
            ColorAlpha(textColor, 0.84F));
        std::snprintf(menuMetric, sizeof(menuMetric), "%d", std::max(playerProfile.totalRuns, 0));
        DrawLocalized(u8"\u5df2\u5b8c\u6210\u5c40\u6570", "RUNS", snapshot.x + 16.0F, snapshot.y + 82.0F, 14.0F, accentSuccess);
        DrawDisplayText(menuMetric, Vector2 {snapshot.x + 16.0F, snapshot.y + 104.0F}, 22.0F, 1.0F, ColorAlpha(textColor, 0.98F));
        std::snprintf(menuMetric, sizeof(menuMetric), "%d", std::max(playerProfile.totalBricksDestroyed, 0));
        DrawLocalized(u8"\u51fb\u788e\u7816\u5757", "BRICKS", snapshot.x + 158.0F, snapshot.y + 82.0F, 14.0F, accentWarm);
        DrawDisplayText(menuMetric, Vector2 {snapshot.x + 158.0F, snapshot.y + 104.0F}, 22.0F, 1.0F, ColorAlpha(textColor, 0.98F));
        std::snprintf(menuMetric, sizeof(menuMetric), "%d", std::max(playerProfile.totalPowerUpsCollected, 0));
        DrawLocalized(u8"\u6536\u96c6\u9053\u5177", "BUFFS", snapshot.x + 300.0F, snapshot.y + 82.0F, 14.0F, accent);
        DrawDisplayText(menuMetric, Vector2 {snapshot.x + 300.0F, snapshot.y + 104.0F}, 22.0F, 1.0F, ColorAlpha(textColor, 0.98F));

        const Rectangle previewField {rightColumn.x + 18.0F, rightColumn.y + 176.0F, rightColumn.width - 36.0F, 112.0F};
        DrawGlassPanel(previewField, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.18F : 0.30F), panelBorder, panelGlow, 0.12F);
        DrawLocalized(
            u8"\u98ce\u683c\u914d\u7f6e",
            "VISUAL LOADOUT",
            previewField.x + 16.0F,
            previewField.y + 10.0F,
            22.0F,
            accentWarm);
        DrawLocalized(
            u8"\u5f53\u524d\u4e3b\u9898\u4e0e\u7403\u4f53\u3001\u6321\u677f\u3001\u7816\u5757\u914d\u8272\u9884\u89c8",
            "Theme, ball, paddle, and brick preview",
            previewField.x + 16.0F,
            previewField.y + 44.0F,
            16.0F,
            ColorAlpha(textColor, 0.82F));
        DrawCircleV(Vector2 {previewField.x + 52.0F, previewField.y + 82.0F}, 14.0F, ballColor);
        DrawCircleV(Vector2 {previewField.x + 52.0F, previewField.y + 82.0F}, 20.0F, ColorAlpha(ballColor, 0.18F));
        DrawRectangleRounded(
            Rectangle {previewField.x + 92.0F, previewField.y + 72.0F, 92.0F, 20.0F},
            0.5F,
            10,
            paddleColor);
        DrawRectangleRounded(
            Rectangle {previewField.x + 216.0F, previewField.y + 64.0F, 74.0F, 26.0F},
            0.20F,
            8,
            brickColor);
        DrawRectangleRounded(
            Rectangle {previewField.x + 302.0F, previewField.y + 70.0F, 46.0F, 18.0F},
            0.50F,
            8,
            ColorAlpha(brickColor, 0.92F));

        const Rectangle styleDock {rightColumn.x + 18.0F, rightColumn.y + 306.0F, rightColumn.width - 36.0F, 156.0F};
        DrawGlassPanel(styleDock, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.18F : 0.30F), panelBorder, panelGlow, 0.12F);
        DrawLocalized(
            u8"\u98ce\u683c\u914d\u7f6e",
            "STYLE SWITCH",
            styleDock.x + 16.0F,
            styleDock.y + 10.0F,
            20.0F,
            accent);

        const auto drawStyleButton = [&](Rectangle rect, const char* title, const char* zhText, Color tint)
        {
            DrawGlassPanel(rect, ColorAlpha(tint, 0.22F), ColorAlpha(tint, 0.72F), ColorAlpha(tint, 0.52F), 0.22F);
            DrawDisplayText(title, Vector2 {rect.x + 14.0F, rect.y + 8.0F}, 12.0F, 1.1F, ColorAlpha(textColor, 0.88F));
            DrawLocalized(zhText, title, rect.x + 14.0F, rect.y + 24.0F, 16.0F, ColorAlpha(textColor, 0.96F));
            DrawCircleV(Vector2 {rect.x + rect.width - 24.0F, rect.y + rect.height * 0.5F}, 9.0F, tint);
        };

        drawStyleButton(
            styleButtons.theme,
            "THEME",
            isDarkMode ? u8"\u754c\u9762\u4e3b\u9898 \u6697\u8272" : u8"\u754c\u9762\u4e3b\u9898 \u4eae\u8272",
            isDarkMode ? Color {154, 169, 188, 255} : Color {81, 121, 176, 255});
        drawStyleButton(styleButtons.ball, "BALL", u8"\u7403\u4f53\u989c\u8272", ballColor);
        drawStyleButton(styleButtons.paddle, "PADDLE", u8"\u6321\u677f\u989c\u8272", paddleColor);
        drawStyleButton(styleButtons.brick, "BRICK", u8"\u7816\u5757\u989c\u8272", brickColor);
    }
    else if (currentState == GameState::PAUSED)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.46F));
        const Rectangle panel {screenWidth / 2.0F - 250.0F, screenHeight / 2.0F - 104.0F, 500.0F, 210.0F};
        DrawGlassPanel(panel, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.60F : 0.82F), panelBorder, panelGlow, 0.12F);
        const char* pausedTitle = "PAUSED";
        const float pausedWidth = MeasureDisplayTextWidth(pausedTitle, 42.0F, 2.0F);
        DrawDisplayText(
            pausedTitle,
            Vector2 {(float)screenWidth / 2.0F - pausedWidth * 0.5F, panel.y + 32.0F},
            42.0F,
            2.0F,
            accentWarm);
        DrawLocalized(
            u8"\u5f53\u524d\u5c40\u5df2\u6682\u505c",
            "Run temporarily halted",
            panel.x + 130.0F,
            panel.y + 92.0F,
            22.0F,
            ColorAlpha(textColor, 0.90F));
        DrawLocalized(
            u8"\u6309 P \u6216\u70b9\u51fb\u53f3\u4e0a\u89d2\u7ee7\u7eed",
            "Press P or use the top-right button to resume",
            panel.x + 72.0F,
            panel.y + 132.0F,
            18.0F,
            subTextColor);
    }
    else if (currentState == GameState::GAMEOVER)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.50F));
        const Rectangle panel {screenWidth / 2.0F - 280.0F, screenHeight / 2.0F - 128.0F, 560.0F, 236.0F};
        DrawGlassPanel(panel, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.62F : 0.84F), panelBorder, panelGlow, 0.12F);
        const char* lostTitle = "RUN LOST";
        const float lostWidth = MeasureDisplayTextWidth(lostTitle, 42.0F, 2.2F);
        DrawDisplayText(
            lostTitle,
            Vector2 {(float)screenWidth / 2.0F - lostWidth * 0.5F, panel.y + 28.0F},
            42.0F,
            2.2F,
            Color {255, 110, 110, 255});
        DrawLocalized(
            u8"\u672c\u5c40\u7ed3\u675f\uff0c\u5df2\u7edf\u8ba1\u8fdb\u6863",
            "This run is over and progression has been recorded",
            panel.x + 78.0F,
            panel.y + 88.0F,
            22.0F,
            ColorAlpha(textColor, 0.92F));
        char finalScore[32] {};
        std::snprintf(finalScore, sizeof(finalScore), "%d", score);
        DrawDisplayText("FINAL SCORE", Vector2 {panel.x + 70.0F, panel.y + 134.0F}, 16.0F, 1.6F, accent);
        DrawDisplayText(finalScore, Vector2 {panel.x + 70.0F, panel.y + 158.0F}, 30.0F, 1.2F, ColorAlpha(textColor, 0.98F));
        std::snprintf(finalScore, sizeof(finalScore), "%d", playerProfile.softCurrency);
        DrawDisplayText("COINS", Vector2 {panel.x + 250.0F, panel.y + 134.0F}, 16.0F, 1.6F, accentWarm);
        DrawDisplayText(finalScore, Vector2 {panel.x + 250.0F, panel.y + 158.0F}, 30.0F, 1.2F, ColorAlpha(textColor, 0.98F));
        std::snprintf(finalScore, sizeof(finalScore), "%d", std::max(playerProfile.bestScore, score));
        DrawDisplayText("BEST", Vector2 {panel.x + 404.0F, panel.y + 134.0F}, 16.0F, 1.6F, accentSuccess);
        DrawDisplayText(finalScore, Vector2 {panel.x + 404.0F, panel.y + 158.0F}, 30.0F, 1.2F, ColorAlpha(textColor, 0.98F));
    }
    else if (victory)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.46F));
        const Rectangle panel {screenWidth / 2.0F - 310.0F, screenHeight / 2.0F - 132.0F, 620.0F, 244.0F};
        DrawGlassPanel(panel, ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.60F : 0.84F), panelBorder, panelGlow, 0.12F);
        const char* clearTitle = "ALL CLEARED";
        const float clearWidth = MeasureDisplayTextWidth(clearTitle, 42.0F, 2.2F);
        DrawDisplayText(
            clearTitle,
            Vector2 {(float)screenWidth / 2.0F - clearWidth * 0.5F, panel.y + 28.0F},
            42.0F,
            2.2F,
            accentSuccess);
        DrawLocalized(
            u8"\u5168\u90e8\u5173\u5361\u5df2\u5b8c\u6210\uff0c\u8fd9\u8f6e\u6210\u7ee9\u5df2\u5165\u5e93",
            "Every stage is cleared and the run has been banked",
            panel.x + 90.0F,
            panel.y + 88.0F,
            22.0F,
            ColorAlpha(textColor, 0.92F));
        char victoryMetric[32] {};
        std::snprintf(victoryMetric, sizeof(victoryMetric), "%d", score);
        DrawDisplayText("FINAL SCORE", Vector2 {panel.x + 82.0F, panel.y + 134.0F}, 16.0F, 1.6F, accent);
        DrawDisplayText(victoryMetric, Vector2 {panel.x + 82.0F, panel.y + 158.0F}, 30.0F, 1.2F, ColorAlpha(textColor, 0.98F));
        std::snprintf(victoryMetric, sizeof(victoryMetric), "%d", playerProfile.softCurrency);
        DrawDisplayText("COINS", Vector2 {panel.x + 280.0F, panel.y + 134.0F}, 16.0F, 1.6F, accentWarm);
        DrawDisplayText(victoryMetric, Vector2 {panel.x + 280.0F, panel.y + 158.0F}, 30.0F, 1.2F, ColorAlpha(textColor, 0.98F));
        std::snprintf(victoryMetric, sizeof(victoryMetric), "%d", std::max(playerProfile.highestStage, currentLevel));
        DrawDisplayText("STAGE", Vector2 {panel.x + 450.0F, panel.y + 134.0F}, 16.0F, 1.6F, accentSuccess);
        DrawDisplayText(victoryMetric, Vector2 {panel.x + 450.0F, panel.y + 158.0F}, 30.0F, 1.2F, ColorAlpha(textColor, 0.98F));
    }

    if (asyncLoadActive)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.28F));
        const Rectangle panel {screenWidth * 0.5F - 310.0F, screenHeight * 0.5F - 118.0F, 620.0F, 236.0F};
        DrawGlassPanel(
            panel,
            ColorAlpha(isDarkMode ? BLACK : WHITE, isDarkMode ? 0.66F : 0.86F),
            ColorAlpha(panelBorder, 0.92F),
            ColorAlpha(panelGlow, 0.94F),
            0.14F);

        DrawLocalized(
            u8"\u5f02\u6b65\u52a0\u8f7d\u6f14\u793a",
            "ASYNC LOAD",
            panel.x + 28.0F,
            panel.y + 28.0F,
            34.0F,
            ColorAlpha(textColor, 0.98F));

        const int dotCount = 1 + ((int)(timeSeconds * 3.0F) % 3);
        const char* loadingDots = dotCount == 1 ? "." : (dotCount == 2 ? ".." : "...");
        DrawDisplayText("Loading", Vector2 {panel.x + 30.0F, panel.y + 70.0F}, 24.0F, 1.4F, accentWarm);
        DrawDisplayText(loadingDots, Vector2 {panel.x + 164.0F, panel.y + 70.0F}, 24.0F, 1.4F, accentWarm);
        DrawLocalized(
            u8"\u540e\u53f0\u7ebf\u7a0b\u6b63\u5728\u51c6\u5907\u8d44\u6e90\uff0c\u4e3b\u7ebf\u7a0b\u7ee7\u7eed\u6e32\u67d3\uff0c\u4e0d\u4f1a\u5361\u4f4f\u754c\u9762",
            "Worker thread is preparing resources while the main thread keeps rendering",
            panel.x + 30.0F,
            panel.y + 108.0F,
            18.0F,
            ColorAlpha(textColor, 0.88F));

        const Rectangle progressBar {panel.x + 30.0F, panel.y + 156.0F, panel.width - 60.0F, 18.0F};
        DrawRectangleRounded(progressBar, 0.45F, 10, ColorAlpha(BLACK, 0.38F));
        Rectangle progressFill = progressBar;
        progressFill.width *= std::clamp(asyncLoadProgressUi, 0.0F, 1.0F);
        DrawRectangleRounded(progressFill, 0.45F, 10, ColorAlpha(accent, 0.95F));
        DrawRectangleRoundedLines(progressBar, 0.45F, 10, 1.4F, ColorAlpha(accent, 0.56F));

        char progressLabel[32] {};
        std::snprintf(progressLabel, sizeof(progressLabel), "%d%%", (int)std::round(std::clamp(asyncLoadProgressUi, 0.0F, 1.0F) * 100.0F));
        DrawLocalized(
            u8"\u5f53\u524d\u8fdb\u5ea6",
            "Progress",
            panel.x + 30.0F,
            panel.y + 186.0F,
            16.0F,
            ColorAlpha(textColor, 0.80F));
        DrawDisplayText(progressLabel, Vector2 {panel.x + panel.width - 86.0F, panel.y + 182.0F}, 18.0F, 1.2F, ColorAlpha(textColor, 0.98F));
    }

    if (asyncLoadSuccessTimer > 0.0F)
    {
        const Rectangle successChip {screenWidth * 0.5F - 232.0F, HUD_HEIGHT + 16.0F, 464.0F, 42.0F};
        DrawGlassPanel(
            successChip,
            ColorAlpha(accentSuccess, 0.20F + 0.08F * std::sin(timeSeconds * 4.2F)),
            ColorAlpha(accentSuccess, 0.84F),
            ColorAlpha(accentSuccess, 0.70F),
            0.32F);
        DrawLocalized(
            u8"\u52a0\u8f7d\u5b8c\u6210\uff0c\u7816\u5757\u4e3b\u9898\u5df2\u5237\u65b0\uff0c\u6309 L \u53ef\u518d\u6b21\u89e6\u53d1",
            "Load finished, press L to trigger again",
            successChip.x + 18.0F,
            successChip.y + 11.0F,
            18.0F,
            ColorAlpha(textColor, 0.98F));
    }

    EndDrawing();
}
