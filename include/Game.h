#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "BallManager.hpp"
#include "Brick.h"
#include "Config/PowerUpConfig.hpp"
#include "GameWorld.hpp"
#include "Paddle.h"
#include "Paddle.hpp"
#include "Particles/ParticleSystem.hpp"
#include "PowerUps/PowerUpSystem.hpp"
#include "raylib.h"

class Game {
private:
    static constexpr float WALL_THICKNESS = 5.0f;
    static constexpr float PADDLE_BASE_SPEED = 8.5f;
    static constexpr int PAUSE_BUTTON_SIZE = 34;
    static constexpr float HUD_HEIGHT = 78.0f;

    struct EdgeParticle {
        Vector2 position {};
        Vector2 velocity {};
        float life {0.0f};
        float maxLife {0.0f};
        float size {0.0f};
        Color color {};
    };

    struct BackgroundPack {
        Texture2D menu {};
        std::vector<Texture2D> levels {};
    };

    enum class GameState { MENU, PLAYING, PAUSED, GAMEOVER };

    int screenWidth;
    int screenHeight;

    Paddle paddle;
    std::vector<Brick> bricks;

    int lives;
    int levelStartLives;
    int score;

    GameState currentState;
    bool victory;
    bool exitWindowRequest;

    bool isDarkMode;
    bool debugMode;

    Color ballColor;
    Color paddleColor;
    Color brickColor;
    int ballColorIndex;
    int paddleColorIndex;
    int brickColorIndex;

    const float PADDLE_INFLUENCE = 0.2f;
    const float MAX_H_SPEED = 10.0f;
    float prevPaddleX;

    int currentLevel;
    std::vector<EdgeParticle> edgeParticles;
    std::vector<std::optional<game::PowerUpType>> brickPowerUps;
    static constexpr int MAX_EDGE_PARTICLES = 600;

    game::BallManager ballManager;
    game::ParticleSystem particleSystem;
    game::PowerUpSystem powerUpSystem;
    game::PowerUpConfigSet powerUpConfigSet;
    game::GameWorld world;
    std::unique_ptr<game::Paddle> effectPaddle;

    Vector2 spawnBallPosition;
    Vector2 spawnBallVelocity;
    float spawnBallRadius;
    float paddleSpeedMultiplier;
    Font uiFont {};
    bool hasUiFont;
    BackgroundPack darkBackgrounds {};
    BackgroundPack lightBackgrounds {};

    void InitConfigAndBricks(const std::string& levelJsonFile);
    void InitBricks();
    void AssignPowerUpsToBricks();
    void LoadPowerUpConfig();
    void LoadBackgroundTextures();
    void UnloadBackgroundTextures();
    void ResetBalls();

    void SyncEffectPaddleToGameplay();
    void ApplyEffectPaddleToGameplay();

    void HandleBalls(float paddleVel);
    void HandleBallEdgeCollision(game::Ball& managedBall);
    void HandleBallPaddleCollision(game::Ball& managedBall, float paddleVel);
    void HandleBallBrickCollision(game::Ball& managedBall);
    void CheckLevelProgress();
    void MaybeSpawnPowerUpFromBrick(std::size_t brickIndex, const Rectangle& brickRect);
    Color GetBrickDisplayColor(std::size_t brickIndex, Color baseColor, float timeSeconds, float globalGlow) const;

    void SpawnEdgeParticles(const Vector2& origin, const Vector2& normal, int count);
    void UpdateEdgeParticles();
    void DrawEdgeParticles() const;
    void DrawPowerUps() const;
    void DrawParticles() const;
    void DrawActiveEffects() const;
    void DrawLocalized(const char* zhText, const char* enText, float x, float y, float fontSize, Color color) const;
    void DrawLocalizedf(const char* zhPrefix, const char* enPrefix, int value, float x, float y, float fontSize, Color color) const;
    float PaddleMinY() const;
    float PaddleMaxY() const;
    const Texture2D* ResolveCurrentBackground() const;

public:
    Game(int width, int height);
    ~Game();
    bool ShouldClose() const;
    void HandleInput();
    void Update();
    void Draw();
};
