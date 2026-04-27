#include "Game.h"

#include <algorithm>
#include <chrono>
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
void writeUserPrefsFile(bool isDarkMode, int ballColorIndex, int paddleColorIndex, int brickColorIndex)
{
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
    codepoints.reserve(512);

    for (int cp = 32; cp <= 126; ++cp)
    {
        codepoints.push_back(cp);
    }

    const char* zhSamples[] {
        u8"\u5f53\u524d\u9053\u5177\u6548\u679c",
        u8"\u5f53\u524d\u6218\u6597\u589e\u76ca",
        u8"\u5e38\u9a7b",
        u8"\u5206\u6570 \u751f\u547d \u7403\u6570 \u5173\u5361",
        u8"\u672c\u5173\u6e05\u7a7a\u81ea\u52a8\u8fdb\u5165\u4e0b\u4e00\u5173",
        u8"\u8c03\u8bd5 \u65e0\u654c",
        u8"\u6253\u7816\u5757 \u7ade\u6280\u7248",
        u8"\u6309 ENTER \u4ece\u7b2c\u4e00\u5173\u5f00\u59cb",
        u8"\u5173\u5361\u4f1a\u81ea\u52a8\u8854\u63a5\uff0c\u9053\u5177\u548c\u591a\u7403\u4f1a\u7ee7\u7eed\u751f\u6548",
        u8"\u4e3b\u9898 \u6697\u8272 \u4eae\u8272 \u7403\u4f53\u914d\u8272 \u6321\u677f\u914d\u8272 \u7816\u5757\u914d\u8272",
        u8"\u5df2\u6682\u505c \u6309 P \u6216\u70b9\u51fb\u53f3\u4e0a\u89d2\u7ee7\u7eed",
        u8"\u5f53\u524d\u5c40\u5df2\u6682\u505c",
        u8"\u6e38\u620f\u7ed3\u675f \u6700\u7ec8\u5206\u6570 \u6309 Q \u9000\u51fa",
        u8"\u672c\u5c40\u7ed3\u675f\uff0c\u5df2\u7edf\u8ba1\u8fdb\u6863",
        u8"\u5168\u90e8\u5173\u5361\u901a\u5173 \u6309 Q \u7ed3\u675f\u672c\u5c40",
        u8"\u5168\u90e8\u5173\u5361\u5df2\u5b8c\u6210\uff0c\u8fd9\u8f6e\u6210\u7ee9\u5df2\u5165\u5e93",
        u8"\u8d85\u7ea7\u957f\u677f \u591a\u7403\u5206\u88c2 \u7403\u901f\u63d0\u5347 \u6321\u677f\u52a0\u901f",
        u8"\u81ea\u52a8\u8854\u63a5\u8fde\u7eed\u95ef\u5173",
        u8"\u8f6f\u8d27\u5e01 \u5386\u53f2\u6700\u9ad8 \u603b\u5c40\u6570",
        u8"\u5546\u4e1a\u5316\u9ad8\u4fdd\u771f\u6253\u7816\u5757\u539f\u578b",
        u8"\u6309 ENTER \u7acb\u5373\u5f00\u59cb\u8fd9\u4e00\u8f6e",
        u8"\u5355\u5c40\u5206\u6570\u3001\u9053\u5177\u53e0\u5c42\u4e0e\u5173\u5361\u8fde\u9501\u5168\u90e8\u4fdd\u7559",
        u8"\u65e5\u5fd7\u5f0f\u6210\u957f\u4e0e\u8f6f\u8d27\u5e01\u7559\u5b58",
        u8"\u4e3b\u9898\u3001\u7403\u4f53\u3001\u6321\u677f\u3001\u7816\u5757\u98ce\u683c\u5feb\u901f\u9884\u89c8",
        u8"\u8857\u673a\u7834\u9635",
        u8"\u5546\u4e1a\u5316\u8857\u673a\u539f\u578b",
        u8"\u9ad8\u4fdd\u771f\u78b0\u649e\u3001\u8fde\u7eed\u95ef\u5173\u548c\u957f\u671f\u6210\u957f\uff0c\u5168\u90e8\u6536\u675f\u5230\u540c\u4e00\u5c40\u4f53\u9a8c",
        u8"\u6309 Enter \u7acb\u5373\u5f00\u5c40\uff0c\u7ee7\u7eed\u4f60\u7684\u4e0b\u4e00\u8f6e\u63a8\u8fdb",
        u8"\u672c\u8f6e\u5165\u53e3",
        u8"\u649e\u7a7f\u7816\u9635\u3001\u7d2f\u79ef\u8f6f\u8d27\u5e01\u3001\u6301\u7eed\u5237\u65b0\u6700\u4f73\u6210\u7ee9",
        u8"\u73a9\u5bb6\u6863\u6848",
        u8"\u672c\u5730\u6210\u957f\u3001\u5c40\u5185\u5f3a\u5ea6\u3001\u5386\u53f2\u6570\u636e\u540c\u65f6\u53ef\u89c1",
        u8"\u5df2\u5b8c\u6210\u5c40\u6570",
        u8"\u51fb\u788e\u7816\u5757",
        u8"\u6536\u96c6\u9053\u5177",
        u8"\u98ce\u683c\u914d\u7f6e",
        u8"\u5f53\u524d\u4e3b\u9898\u4e0e\u7403\u4f53\u3001\u6321\u677f\u3001\u7816\u5757\u914d\u8272\u9884\u89c8",
        u8"\u754c\u9762\u4e3b\u9898",
        u8"\u7403\u4f53\u989c\u8272",
        u8"\u6321\u677f\u989c\u8272",
        u8"\u7816\u5757\u989c\u8272",
        u8"\u6309 L \u6f14\u793a\u5f02\u6b65\u52a0\u8f7d\uff0c\u5f55\u5c4f\u65f6\u53ef\u76f4\u63a5\u5c55\u793a\u4e0d\u5361\u987f\u7684 Loading \u52a8\u753b",
        u8"\u5f02\u6b65\u52a0\u8f7d\u6f14\u793a",
        u8"\u540e\u53f0\u7ebf\u7a0b\u6b63\u5728\u51c6\u5907\u8d44\u6e90\uff0c\u4e3b\u7ebf\u7a0b\u7ee7\u7eed\u6e32\u67d3\uff0c\u4e0d\u4f1a\u5361\u4f4f\u754c\u9762",
        u8"\u52a0\u8f7d\u5b8c\u6210\uff0c\u7816\u5757\u4e3b\u9898\u5df2\u5237\u65b0",
        u8"\u6309 L \u53ef\u518d\u6b21\u89e6\u53d1",
        u8"\u5f53\u524d\u8fdb\u5ea6",
        u8"\u8d44\u6e90\u70ed\u52a0\u8f7d",
        u8"\u52a0\u8f7d\u671f\u95f4\u4ecd\u53ef\u7ee7\u7eed\u6e32\u67d3\u548c\u64cd\u4f5c",
    };

    for (const char* sample : zhSamples)
    {
        appendUniqueCodepoints(sample, codepoints);
    }

    return codepoints;
}

std::vector<int> buildDisplayFontCodepoints()
{
    std::vector<int> codepoints;
    codepoints.reserve(96);
    for (int cp = 32; cp <= 126; ++cp)
    {
        codepoints.push_back(cp);
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
    , prevPaddlePosition {}
    , currentLevel(1)
    , particleSystem(512U)
    , spawnBallPosition {(float)width / 2.0F, (float)height / 2.0F}
    , spawnBallVelocity {240.0F, 240.0F}
    , spawnBallRadius(15.0F)
    , paddleSpeedMultiplier(1.0F)
    , pendingPierceCharges(0)
    , playerProfile {}
    , profileSaveAccumulator(0.0F)
    , hasUiFont(false)
    , hasDisplayFont(false)
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
    world.pierceCharges = &pendingPierceCharges;
    world.playerProfile = &playerProfile;

    const char* fontCandidates[] {
        "C:/Windows/Fonts/msyhbd.ttc",
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
    };
    const std::vector<int> uiCodepoints = buildUiFontCodepoints();
    const std::vector<int> displayCodepoints = buildDisplayFontCodepoints();
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

    Font loadedDisplayFont
        = LoadFontEx("assets/fonts/Orbitron-Bold.ttf", 56, const_cast<int*>(displayCodepoints.data()), (int)displayCodepoints.size());
    if (loadedDisplayFont.texture.id != 0)
    {
        displayFont = loadedDisplayFont;
        hasDisplayFont = true;
    }

    LoadPowerUpConfig();
    LoadPlayerProfile();
    LoadBackgroundTextures();
    currentLevel = 1;
    InitConfigAndBricks(game_style::levelConfigs().front());
    ResetBalls();
    const Rectangle initialPaddleRect = paddle.GetRect();
    prevPaddlePosition = Vector2 {initialPaddleRect.x, initialPaddleRect.y};
}

Game::~Game()
{
    JoinAsyncLoadThread();
    SavePlayerProfile();
    UnloadBackgroundTextures();
    if (hasUiFont && uiFont.texture.id != 0)
    {
        UnloadFont(uiFont);
    }
    if (hasDisplayFont && displayFont.texture.id != 0)
    {
        UnloadFont(displayFont);
    }
}

void Game::LoadPlayerProfile()
{
    playerProfile = game::PlayerProfileStore::load("player_profile.json");
    ++playerProfile.totalSessions;
    SavePlayerProfile();
}

void Game::SavePlayerProfile()
{
    playerProfile.totalPlaySeconds += (int)profileSaveAccumulator;
    profileSaveAccumulator = 0.0F;
    playerProfile.bestScore = std::max(playerProfile.bestScore, score);
    playerProfile.highestStage = std::max(playerProfile.highestStage, currentLevel);
    (void)game::PlayerProfileStore::save("player_profile.json", playerProfile);
}

void Game::RegisterBrickDestroyed(int durability)
{
    playerProfile.totalBricksDestroyed += 1;
    playerProfile.softCurrency += std::max(1, durability);
    playerProfile.lifetimeScore += 100 * std::max(1, durability);
    playerProfile.bestScore = std::max(playerProfile.bestScore, score);
    playerProfile.highestStage = std::max(playerProfile.highestStage, currentLevel);
}

void Game::RegisterPowerUpCollected()
{
    ++playerProfile.totalPowerUpsCollected;
}

void Game::FinalizeRunProgress()
{
    playerProfile.bestScore = std::max(playerProfile.bestScore, score);
    playerProfile.highestStage = std::max(playerProfile.highestStage, currentLevel);
    SavePlayerProfile();
}

void Game::JoinAsyncLoadThread()
{
    if (asyncLoadThread.joinable())
    {
        asyncLoadThread.join();
    }
}

void Game::StartAsyncLoadDemo()
{
    if (asyncLoadActive)
    {
        return;
    }

    JoinAsyncLoadThread();

    const auto& palette = game_style::colorPalette();
    const int paletteSize = (int)palette.size();
    const int preparedBrickColorIndex = (brickColorIndex + 2 + GetRandomValue(0, 2)) % std::max(paletteSize, 1);
    const bool preparedDarkMode = !isDarkMode;

    {
        std::lock_guard<std::mutex> guard(asyncLoadMutex);
        asyncLoadShared = AsyncLoadSharedState {};
        asyncLoadShared.preparedBrickColorIndex = preparedBrickColorIndex;
        asyncLoadShared.preparedDarkMode = preparedDarkMode;
    }

    asyncLoadActive = true;
    asyncLoadProgressUi = 0.0F;
    asyncLoadSuccessTimer = 0.0F;

    asyncLoadThread = std::thread(
        [this, preparedBrickColorIndex, preparedDarkMode]()
        {
            constexpr int steps = 48;
            for (int step = 1; step <= steps; ++step)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(55));
                std::lock_guard<std::mutex> guard(asyncLoadMutex);
                asyncLoadShared.progress = (float)step / (float)steps;
                asyncLoadShared.preparedBrickColorIndex = preparedBrickColorIndex;
                asyncLoadShared.preparedDarkMode = preparedDarkMode;
            }

            std::lock_guard<std::mutex> guard(asyncLoadMutex);
            asyncLoadShared.progress = 1.0F;
            asyncLoadShared.preparedBrickColorIndex = preparedBrickColorIndex;
            asyncLoadShared.preparedDarkMode = preparedDarkMode;
            asyncLoadShared.finished = true;
        });
}

void Game::PollAsyncLoadDemo(float deltaSeconds)
{
    if (asyncLoadSuccessTimer > 0.0F)
    {
        asyncLoadSuccessTimer = std::max(0.0F, asyncLoadSuccessTimer - deltaSeconds);
    }

    if (!asyncLoadActive)
    {
        return;
    }

    AsyncLoadSharedState snapshot {};
    {
        std::lock_guard<std::mutex> guard(asyncLoadMutex);
        snapshot = asyncLoadShared;
    }

    asyncLoadProgressUi = snapshot.progress;
    if (!snapshot.finished)
    {
        return;
    }

    JoinAsyncLoadThread();
    asyncLoadActive = false;
    asyncLoadProgressUi = 1.0F;
    asyncLoadSuccessTimer = 3.6F;

    isDarkMode = snapshot.preparedDarkMode;
    brickColorIndex = snapshot.preparedBrickColorIndex;

    const auto& palette = game_style::colorPalette();
    brickColor = palette[(std::size_t)brickColorIndex % palette.size()];
    ballColor = palette[(std::size_t)ballColorIndex % palette.size()];
    paddleColor = palette[(std::size_t)paddleColorIndex % palette.size()];

    writeUserPrefsFile(isDarkMode, ballColorIndex, paddleColorIndex, brickColorIndex);
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
        powerUpConfigSet.powerUps[game::PowerUpType::PierceBall]
            = game::PowerUpConfig {game::PowerUpType::PierceBall, 0.18F, 0.0F, 4.0F, true};
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

Game::MenuStyleButtonRects Game::MenuStyleButtons() const
{
    const Rectangle shell {60.0F, (float)screenHeight * 0.16F, (float)screenWidth - 120.0F, (float)screenHeight * 0.64F};
    const Rectangle leftColumn {shell.x + 18.0F, shell.y + 18.0F, shell.width * 0.53F, shell.height - 36.0F};
    const Rectangle rightColumn {
        leftColumn.x + leftColumn.width + 18.0F,
        shell.y + 18.0F,
        shell.width - leftColumn.width - 36.0F,
        shell.height - 36.0F,
    };

    const Rectangle buttonArea {rightColumn.x + 18.0F, rightColumn.y + 338.0F, rightColumn.width - 36.0F, 118.0F};
    const float gap = 14.0F;
    const float buttonWidth = (buttonArea.width - gap) * 0.5F;
    const float buttonHeight = 52.0F;

    return MenuStyleButtonRects {
        Rectangle {buttonArea.x, buttonArea.y, buttonWidth, buttonHeight},
        Rectangle {buttonArea.x + buttonWidth + gap, buttonArea.y, buttonWidth, buttonHeight},
        Rectangle {buttonArea.x, buttonArea.y + buttonHeight + gap, buttonWidth, buttonHeight},
        Rectangle {buttonArea.x + buttonWidth + gap, buttonArea.y + buttonHeight + gap, buttonWidth, buttonHeight},
    };
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

    if (currentState != GameState::PAUSED && IsKeyPressed(KEY_L))
    {
        StartAsyncLoadDemo();
    }

    if (currentState == GameState::MENU)
    {
        const MenuStyleButtonRects styleButtons = MenuStyleButtons();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (CheckCollisionPointRec(mousePos, styleButtons.theme))
            {
                isDarkMode = !isDarkMode;
            }
            if (CheckCollisionPointRec(mousePos, styleButtons.ball))
            {
                ballColorIndex = (ballColorIndex + 1) % 8;
            }
            if (CheckCollisionPointRec(mousePos, styleButtons.paddle))
            {
                paddleColorIndex = (paddleColorIndex + 1) % 8;
            }
            if (CheckCollisionPointRec(mousePos, styleButtons.brick))
            {
                brickColorIndex = (brickColorIndex + 1) % 8;
            }
        }

        const auto& palette = game_style::colorPalette();
        ballColor = palette[(std::size_t)ballColorIndex % palette.size()];
        paddleColor = palette[(std::size_t)paddleColorIndex % palette.size()];
        brickColor = palette[(std::size_t)brickColorIndex % palette.size()];

        if (IsKeyPressed(KEY_ENTER) && !asyncLoadActive)
        {
            currentState = GameState::PLAYING;
            victory = false;
            score = 0;
            currentLevel = 1;
            lives = levelStartLives;
            powerUpSystem.clear(world);
            edgeParticles.clear();
            particleSystem.clear();
            pendingPierceCharges = 0;
            ++playerProfile.totalRuns;
            InitConfigAndBricks(game_style::levelConfigs().front());
            ResetBalls();
            const Rectangle resetPaddleRect = paddle.GetRect();
            prevPaddlePosition = Vector2 {resetPaddleRect.x, resetPaddleRect.y};
            SavePlayerProfile();
            writeUserPrefsFile(isDarkMode, ballColorIndex, paddleColorIndex, brickColorIndex);
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
            const float deltaSeconds = std::max(GetFrameTime(), 0.0001F);
            const float paddleMoveDistance
                = PADDLE_BASE_SPEED_PIXELS_PER_SECOND * std::max(1.0F, paddleSpeedMultiplier) * deltaSeconds;
            const float paddleTopLimit = PaddleMinY();
            const float paddleBottomLimit = PaddleMaxY();
            if (IsKeyDown(KEY_LEFT))
            {
                paddle.MoveLeft(paddleMoveDistance);
            }
            if (IsKeyDown(KEY_RIGHT))
            {
                paddle.MoveRight(paddleMoveDistance);
            }
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
            {
                paddle.MoveUp(paddleMoveDistance, paddleTopLimit);
            }
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
            {
                paddle.MoveDown(paddleMoveDistance, paddleBottomLimit);
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
    const float deltaSeconds = std::max(GetFrameTime(), 0.0001F);
    PollAsyncLoadDemo(deltaSeconds);

    if (currentState != GameState::PLAYING || victory)
    {
        UpdateEdgeParticles();
        return;
    }

    profileSaveAccumulator += deltaSeconds;
    if (profileSaveAccumulator >= 15.0F)
    {
        playerProfile.totalPlaySeconds += (int)profileSaveAccumulator;
        profileSaveAccumulator = 0.0F;
        SavePlayerProfile();
    }

    const Rectangle paddleRect = paddle.GetRect();
    const game::Vec2 paddleVelocity {
        (paddleRect.x - prevPaddlePosition.x) / deltaSeconds,
        (paddleRect.y - prevPaddlePosition.y) / deltaSeconds,
    };
    SyncEffectPaddleToGameplay();
    HandleBalls(paddleVelocity, deltaSeconds);

    powerUpSystem.update(deltaSeconds, world, (float)screenHeight);
    particleSystem.update(deltaSeconds);

    ApplyEffectPaddleToGameplay();
    UpdateEdgeParticles();
    const Rectangle updatedPaddleRect = paddle.GetRect();
    prevPaddlePosition = Vector2 {updatedPaddleRect.x, updatedPaddleRect.y};
}
