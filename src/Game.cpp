#include "Game.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cctype>
#include <vector>

#include "Config/PowerUpConfigLoader.hpp"
#include "GameStyle.hpp"
#include "nlohmann_json.hpp"

using json = nlohmann::json;

namespace
{
void appendUniqueCodepoints(const char* text, std::vector<int>& codepoints)
{
    if (text == nullptr)
    {
        return;
    }

    const char* walker = text;
    while (*walker != '\0')
    {
        int bytes = 0;
        const int cp = GetCodepointNext(walker, &bytes);
        if (cp > 0 && std::find(codepoints.begin(), codepoints.end(), cp) == codepoints.end())
        {
            codepoints.push_back(cp);
        }
        walker += (bytes > 0 ? bytes : 1);
    }
}

std::vector<int> buildUiFontCodepoints()
{
    std::vector<int> codepoints;
    codepoints.reserve(256);

    for (int cp = 32; cp <= 126; ++cp)
    {
        codepoints.push_back(cp);
    }

    const char* zhSamples[] {
        u8"\u5f53\u524d\u9053\u5177\u6548\u679c",
        u8"\u5e38\u9a7b",
        u8"\u5206\u6570 \u751f\u547d \u7403\u6570 \u5173\u5361",
        u8"\u672c\u5173\u6e05\u7a7a\u81ea\u52a8\u8fdb\u5165\u4e0b\u4e00\u5173",
        u8"\u8c03\u8bd5 \u65e0\u654c",
        u8"\u6253\u7816\u5757 \u7ade\u6280\u7248",
        u8"\u6309 ENTER \u4ece\u7b2c\u4e00\u5173\u5f00\u59cb",
        u8"\u5173\u5361\u4f1a\u81ea\u52a8\u8854\u63a5\uff0c\u9053\u5177\u548c\u591a\u7403\u4f1a\u7ee7\u7eed\u751f\u6548",
        u8"\u4e3b\u9898 \u6697\u8272 \u4eae\u8272 \u7403\u4f53\u914d\u8272 \u6321\u677f\u914d\u8272 \u7816\u5757\u914d\u8272",
        u8"\u5df2\u6682\u505c \u6309 P \u6216\u70b9\u51fb\u53f3\u4e0a\u89d2\u7ee7\u7eed",
        u8"\u6e38\u620f\u7ed3\u675f \u6700\u7ec8\u5206\u6570 \u6309 Q \u9000\u51fa",
        u8"\u5168\u90e8\u5173\u5361\u901a\u5173 \u6309 Q \u7ed3\u675f\u672c\u5c40",
        u8"\u8d85\u7ea7\u957f\u677f \u591a\u7403\u5206\u88c2 \u7403\u901f\u63d0\u5347 \u6321\u677f\u52a0\u901f",
    };

    for (const char* sample : zhSamples)
    {
        appendUniqueCodepoints(sample, codepoints);
    }

    return codepoints;
}

struct BackgroundCandidate
{
    std::string path {};
    float luminance {128.0F};
};

float sampleImageLuminance(const Image& image)
{
    if (image.data == nullptr || image.width <= 0 || image.height <= 0)
    {
        return 128.0F;
    }

    Color* pixels = LoadImageColors(image);
    if (pixels == nullptr)
    {
        return 128.0F;
    }

    const int pixelCount = image.width * image.height;
    const int step = std::max(1, pixelCount / 60000);
    double sum = 0.0;
    int samples = 0;
    for (int index = 0; index < pixelCount; index += step)
    {
        const Color color = pixels[index];
        sum += 0.2126 * (double)color.r + 0.7152 * (double)color.g + 0.0722 * (double)color.b;
        ++samples;
    }

    UnloadImageColors(pixels);
    if (samples <= 0)
    {
        return 128.0F;
    }

    return (float)(sum / (double)samples);
}

std::vector<std::string> collectImagePaths(const std::string& folderPath)
{
    namespace fs = std::filesystem;
    std::vector<std::string> paths;

    std::error_code ec;
    if (!fs::exists(folderPath, ec))
    {
        return paths;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(folderPath, ec))
    {
        if (ec || !entry.is_regular_file())
        {
            continue;
        }

        std::string extension = entry.path().extension().string();
        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char ch)
            {
                return (char)std::tolower(ch);
            });

        if (extension == ".jpg" || extension == ".jpeg" || extension == ".png" || extension == ".webp")
        {
            paths.push_back(entry.path().string());
        }
    }

    std::sort(paths.begin(), paths.end());
    return paths;
}
} // namespace

Game::Game(int width, int height)
    : screenWidth(width)
    , screenHeight(height)
    , paddle((float)width / 2.0F - 75.0F, (float)height - 68.0F, 150.0F, 25.0F)
    , lives(3)
    , levelStartLives(3)
    , score(0)
    , currentState(GameState::MENU)
    , victory(false)
    , exitWindowRequest(false)
    , isDarkMode(false)
    , debugMode(false)
    , ballColor(RED)
    , paddleColor(BLUE)
    , brickColor(GREEN)
    , ballColorIndex(0)
    , paddleColorIndex(1)
    , brickColorIndex(2)
    , prevPaddleX(0.0F)
    , currentLevel(1)
    , particleSystem(512U)
    , spawnBallPosition {(float)width / 2.0F, (float)height / 2.0F}
    , spawnBallVelocity {240.0F, 240.0F}
    , spawnBallRadius(15.0F)
    , paddleSpeedMultiplier(1.0F)
    , hasUiFont(false)
{
    std::ifstream prefsFile("user_prefs.json");
    if (prefsFile.is_open())
    {
        try
        {
            const json prefs = json::parse(prefsFile, nullptr, true, true);
            isDarkMode = prefs.value("isDarkMode", false);
            ballColorIndex = prefs.value("ballColorIndex", 0);
            paddleColorIndex = prefs.value("paddleColorIndex", 1);
            brickColorIndex = prefs.value("brickColorIndex", 2);
        }
        catch (const std::exception&)
        {
        }
    }

    const auto& palette = game_style::colorPalette();
    ballColor = palette[(std::size_t)ballColorIndex % palette.size()];
    paddleColor = palette[(std::size_t)paddleColorIndex % palette.size()];
    brickColor = palette[(std::size_t)brickColorIndex % palette.size()];

    effectPaddle = std::make_unique<game::Paddle>(game_style::toGameRect(paddle.GetRect()));
    world.paddle = effectPaddle.get();
    world.ballManager = &ballManager;
    world.particleSystem = &particleSystem;
    world.paddleSpeedMultiplier = &paddleSpeedMultiplier;

    const char* fontCandidates[] {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
    };
    const std::vector<int> uiCodepoints = buildUiFontCodepoints();
    for (const char* fontPath : fontCandidates)
    {
        Font loaded = LoadFontEx(fontPath, 40, const_cast<int*>(uiCodepoints.data()), (int)uiCodepoints.size());
        if (loaded.texture.id != 0)
        {
            uiFont = loaded;
            hasUiFont = true;
            break;
        }
    }

    LoadPowerUpConfig();
    LoadBackgroundTextures();
    currentLevel = 1;
    InitConfigAndBricks(game_style::levelConfigs().front());
    ResetBalls();
    prevPaddleX = paddle.GetRect().x;
}

Game::~Game()
{
    UnloadBackgroundTextures();
    if (hasUiFont && uiFont.texture.id != 0)
    {
        UnloadFont(uiFont);
    }
}

void Game::InitConfigAndBricks(const std::string& levelJsonFile)
{
    bricks.clear();
    edgeParticles.clear();

    int configuredLives = 3;
    Vector2 configuredBallPos {(float)screenWidth / 2.0F, (float)screenHeight / 2.0F};
    Vector2 configuredBallSpeed {4.0F, 4.0F};
    float configuredBallRadius = 15.0F;
    Rectangle configuredPaddle {
        (float)screenWidth / 2.0F - 75.0F,
        (float)screenHeight - 68.0F,
        150.0F,
        25.0F,
    };

    std::ifstream input(levelJsonFile);
    if (input.is_open())
    {
        try
        {
            const json data = json::parse(input, nullptr, true, true);

            if (data.contains("game") && data["game"].is_object())
            {
                configuredLives = data["game"].value("lives", configuredLives);
            }

            if (data.contains("ball") && data["ball"].is_object())
            {
                configuredBallPos.x = data["ball"].value("startX", configuredBallPos.x);
                configuredBallPos.y = data["ball"].value("startY", configuredBallPos.y);
                configuredBallSpeed.x = data["ball"].value("speedX", configuredBallSpeed.x);
                configuredBallSpeed.y = data["ball"].value("speedY", configuredBallSpeed.y);
                configuredBallRadius = data["ball"].value("radius", configuredBallRadius);
            }

            if (data.contains("paddle") && data["paddle"].is_object())
            {
                configuredPaddle.x = data["paddle"].value("startX", configuredPaddle.x);
                configuredPaddle.y = data["paddle"].value("startY", configuredPaddle.y);
                configuredPaddle.width = data["paddle"].value("width", configuredPaddle.width);
                configuredPaddle.height = data["paddle"].value("height", configuredPaddle.height);
            }
        }
        catch (const json::parse_error& error)
        {
            std::cerr << "JSON parse error: " << error.what() << std::endl;
        }
    }
    else
    {
        std::cerr << "Could not open level config file: " << levelJsonFile << std::endl;
    }

    levelStartLives = configuredLives;
    if (currentState == GameState::MENU || currentState == GameState::GAMEOVER)
    {
        lives = levelStartLives;
    }

    paddle = Paddle(configuredPaddle.x, configuredPaddle.y, configuredPaddle.width, configuredPaddle.height);
    effectPaddle = std::make_unique<game::Paddle>(game_style::toGameRect(configuredPaddle));
    world.paddle = effectPaddle.get();

    spawnBallPosition = configuredBallPos;
    spawnBallVelocity = Vector2 {configuredBallSpeed.x * 60.0F, configuredBallSpeed.y * 60.0F};
    spawnBallRadius = std::clamp(configuredBallRadius * 1.35F, 18.0F, 28.0F);
    paddleSpeedMultiplier = 1.0F;

    InitBricks();
    SyncEffectPaddleToGameplay();
    ApplyEffectPaddleToGameplay();
}

void Game::LoadPowerUpConfig()
{
    game::PowerUpConfigLoader loader;
    try
    {
        powerUpConfigSet = loader.loadFromFile("config/powerups.json");
    }
    catch (const std::exception& error)
    {
        std::cerr << "Power-up config load failed, fallback defaults: " << error.what() << std::endl;
        powerUpConfigSet = game::PowerUpConfigSet {};
        powerUpConfigSet.particles = game::ParticleConfig {};
        powerUpConfigSet.powerUps[game::PowerUpType::ExpandPaddle]
            = game::PowerUpConfig {game::PowerUpType::ExpandPaddle, 0.34F, 18.0F, 2.2F, false};
        powerUpConfigSet.powerUps[game::PowerUpType::MultiBall]
            = game::PowerUpConfig {game::PowerUpType::MultiBall, 0.20F, 0.0F, 3.0F, true};
        powerUpConfigSet.powerUps[game::PowerUpType::SlowBall]
            = game::PowerUpConfig {game::PowerUpType::SlowBall, 0.26F, 12.0F, 1.35F, false};
        powerUpConfigSet.powerUps[game::PowerUpType::PaddleSpeed]
            = game::PowerUpConfig {game::PowerUpType::PaddleSpeed, 0.24F, 14.0F, 1.7F, false};
    }

    powerUpSystem.setConfigSet(powerUpConfigSet);

    particleSystem = game::ParticleSystem(powerUpConfigSet.particles.maxCount);
    particleSystem.setReflectionBounds(game::Rect {
        WALL_THICKNESS,
        WALL_THICKNESS,
        (float)screenWidth - WALL_THICKNESS * 2.0F,
        (float)screenHeight - WALL_THICKNESS * 2.0F,
    });
    particleSystem.setEdgeReflectionEnabled(true);
    particleSystem.setReflectionDamping(0.78F);
    world.particleSystem = &particleSystem;
}

void Game::LoadBackgroundTextures()
{
    UnloadBackgroundTextures();

    const std::vector<std::string> imagePaths = collectImagePaths("image");
    if (imagePaths.empty())
    {
        return;
    }

    std::vector<BackgroundCandidate> darkCandidates;
    std::vector<BackgroundCandidate> lightCandidates;
    darkCandidates.reserve(imagePaths.size());
    lightCandidates.reserve(imagePaths.size());

    for (const std::string& path : imagePaths)
    {
        Image image = LoadImage(path.c_str());
        if (image.data == nullptr)
        {
            continue;
        }

        const float luminance = sampleImageLuminance(image);
        UnloadImage(image);

        BackgroundCandidate candidate {path, luminance};
        if (luminance <= 136.0F)
        {
            darkCandidates.push_back(candidate);
        }
        if (luminance >= 108.0F)
        {
            lightCandidates.push_back(candidate);
        }
    }

    if (darkCandidates.empty() || lightCandidates.empty())
    {
        for (const std::string& path : imagePaths)
        {
            Image image = LoadImage(path.c_str());
            if (image.data == nullptr)
            {
                continue;
            }
            const float luminance = sampleImageLuminance(image);
            UnloadImage(image);
            BackgroundCandidate candidate {path, luminance};
            darkCandidates.push_back(candidate);
            lightCandidates.push_back(candidate);
        }
    }

    std::sort(
        darkCandidates.begin(),
        darkCandidates.end(),
        [](const BackgroundCandidate& lhs, const BackgroundCandidate& rhs)
        {
            return lhs.luminance < rhs.luminance;
        });
    std::sort(
        lightCandidates.begin(),
        lightCandidates.end(),
        [](const BackgroundCandidate& lhs, const BackgroundCandidate& rhs)
        {
            return lhs.luminance > rhs.luminance;
        });

    const std::size_t levelCount = game_style::levelConfigs().size();
    auto loadPack = [levelCount](const std::vector<BackgroundCandidate>& candidates, BackgroundPack& pack)
    {
        if (candidates.empty())
        {
            return;
        }

        const Texture2D menuTexture = LoadTexture(candidates[0].path.c_str());
        if (menuTexture.id != 0)
        {
            pack.menu = menuTexture;
        }

        pack.levels.reserve(levelCount);
        for (std::size_t levelIndex = 0; levelIndex < levelCount; ++levelIndex)
        {
            const std::size_t pick = (levelIndex + 1) % candidates.size();
            const Texture2D levelTexture = LoadTexture(candidates[pick].path.c_str());
            if (levelTexture.id != 0)
            {
                pack.levels.push_back(levelTexture);
            }
        }
    };

    loadPack(darkCandidates, darkBackgrounds);
    loadPack(lightCandidates, lightBackgrounds);
}

void Game::UnloadBackgroundTextures()
{
    auto unloadPack = [](BackgroundPack& pack)
    {
        if (pack.menu.id != 0)
        {
            UnloadTexture(pack.menu);
            pack.menu = Texture2D {};
        }

        for (Texture2D& texture : pack.levels)
        {
            if (texture.id != 0)
            {
                UnloadTexture(texture);
            }
        }
        pack.levels.clear();
    };

    unloadPack(darkBackgrounds);
    unloadPack(lightBackgrounds);
}

const Texture2D* Game::ResolveCurrentBackground() const
{
    const BackgroundPack& pack = isDarkMode ? darkBackgrounds : lightBackgrounds;
    if (currentState == GameState::MENU)
    {
        return pack.menu.id != 0 ? &pack.menu : nullptr;
    }

    if (pack.levels.empty())
    {
        return nullptr;
    }

    const std::size_t levelIndex = (std::size_t)std::max(currentLevel - 1, 0);
    const std::size_t pick = std::min(levelIndex, pack.levels.size() - 1);
    return &pack.levels[pick];
}

bool Game::ShouldClose() const
{
    return exitWindowRequest || WindowShouldClose();
}

void Game::HandleInput()
{
    const Vector2 mousePos = GetMousePosition();
    if (IsKeyPressed(KEY_Q))
    {
        exitWindowRequest = true;
    }

    if (IsKeyPressed(KEY_Z))
    {
        debugMode = !debugMode;
    }

    if (currentState == GameState::MENU)
    {
        const Rectangle themeBtn {screenWidth / 2.0F - 235.0F, screenHeight / 2.0F + 80.0F, 210.0F, 52.0F};
        const Rectangle ballBtn {screenWidth / 2.0F + 25.0F, screenHeight / 2.0F + 80.0F, 210.0F, 52.0F};
        const Rectangle paddleBtn {screenWidth / 2.0F - 235.0F, screenHeight / 2.0F + 150.0F, 210.0F, 52.0F};
        const Rectangle brickBtn {screenWidth / 2.0F + 25.0F, screenHeight / 2.0F + 150.0F, 210.0F, 52.0F};

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (CheckCollisionPointRec(mousePos, themeBtn))
            {
                isDarkMode = !isDarkMode;
            }
            if (CheckCollisionPointRec(mousePos, ballBtn))
            {
                ballColorIndex = (ballColorIndex + 1) % 8;
            }
            if (CheckCollisionPointRec(mousePos, paddleBtn))
            {
                paddleColorIndex = (paddleColorIndex + 1) % 8;
            }
            if (CheckCollisionPointRec(mousePos, brickBtn))
            {
                brickColorIndex = (brickColorIndex + 1) % 8;
            }
        }

        const auto& palette = game_style::colorPalette();
        ballColor = palette[(std::size_t)ballColorIndex % palette.size()];
        paddleColor = palette[(std::size_t)paddleColorIndex % palette.size()];
        brickColor = palette[(std::size_t)brickColorIndex % palette.size()];

        if (IsKeyPressed(KEY_ENTER))
        {
            currentState = GameState::PLAYING;
            victory = false;
            score = 0;
            currentLevel = 1;
            lives = levelStartLives;
            powerUpSystem.clear(world);
            edgeParticles.clear();
            particleSystem.clear();
            InitConfigAndBricks(game_style::levelConfigs().front());
            ResetBalls();
            prevPaddleX = paddle.GetRect().x;

            json prefs;
            prefs["isDarkMode"] = isDarkMode;
            prefs["ballColorIndex"] = ballColorIndex;
            prefs["paddleColorIndex"] = paddleColorIndex;
            prefs["brickColorIndex"] = brickColorIndex;
            std::ofstream out("user_prefs.json");
            if (out.is_open())
            {
                out << prefs.dump(4);
            }
        }
    }
    else if (currentState == GameState::PLAYING)
    {
        const Rectangle pauseBtn = game_style::pauseButtonRect(screenWidth, PAUSE_BUTTON_SIZE);
        if (IsKeyPressed(KEY_P))
        {
            currentState = GameState::PAUSED;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, pauseBtn))
        {
            currentState = GameState::PAUSED;
        }

        if (!victory)
        {
            const float paddleMoveSpeed = PADDLE_BASE_SPEED * std::max(1.0F, paddleSpeedMultiplier);
            const float paddleTopLimit = PaddleMinY();
            const float paddleBottomLimit = PaddleMaxY();
            if (IsKeyDown(KEY_LEFT))
            {
                paddle.MoveLeft(paddleMoveSpeed);
            }
            if (IsKeyDown(KEY_RIGHT))
            {
                paddle.MoveRight(paddleMoveSpeed);
            }
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
            {
                paddle.MoveUp(paddleMoveSpeed, paddleTopLimit);
            }
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
            {
                paddle.MoveDown(paddleMoveSpeed, paddleBottomLimit);
            }
            SyncEffectPaddleToGameplay();
            ApplyEffectPaddleToGameplay();
        }
    }
    else if (currentState == GameState::PAUSED)
    {
        const Rectangle resumeBtn = game_style::pauseButtonRect(screenWidth, PAUSE_BUTTON_SIZE);
        if (IsKeyPressed(KEY_P))
        {
            currentState = GameState::PLAYING;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, resumeBtn))
        {
            currentState = GameState::PLAYING;
        }
    }
}

void Game::Update()
{
    if (currentState != GameState::PLAYING || victory)
    {
        UpdateEdgeParticles();
        return;
    }

    const float paddleVel = paddle.GetRect().x - prevPaddleX;
    SyncEffectPaddleToGameplay();
    HandleBalls(paddleVel);

    const float deltaSeconds = std::max(GetFrameTime(), 0.0001F);
    powerUpSystem.update(deltaSeconds, world, (float)screenHeight);
    particleSystem.update(deltaSeconds);

    ApplyEffectPaddleToGameplay();
    UpdateEdgeParticles();
    prevPaddleX = paddle.GetRect().x;
}
