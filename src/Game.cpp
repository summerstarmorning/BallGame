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
constexpr const char* kRunSavePath = "save/run_save.json";
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
        u8"\u6309\u56de\u8f66\u952e\u4ece\u7b2c\u4e00\u5173\u5f00\u59cb",
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
        u8"\u6309\u56de\u8f66\u952e\u7acb\u5373\u5f00\u59cb\u8fd9\u4e00\u8f6e",
        u8"\u5355\u5c40\u5206\u6570\u3001\u9053\u5177\u53e0\u5c42\u4e0e\u5173\u5361\u8fde\u9501\u5168\u90e8\u4fdd\u7559",
        u8"\u65e5\u5fd7\u5f0f\u6210\u957f\u4e0e\u8f6f\u8d27\u5e01\u7559\u5b58",
        u8"\u4e3b\u9898\u3001\u7403\u4f53\u3001\u6321\u677f\u3001\u7816\u5757\u98ce\u683c\u5feb\u901f\u9884\u89c8",
        u8"\u8857\u673a\u7834\u9635",
        u8"\u5546\u4e1a\u5316\u8857\u673a\u539f\u578b",
        u8"\u9ad8\u4fdd\u771f\u78b0\u649e\u3001\u8fde\u7eed\u95ef\u5173\u548c\u957f\u671f\u6210\u957f\uff0c\u5168\u90e8\u6536\u675f\u5230\u540c\u4e00\u5c40\u4f53\u9a8c",
        u8"\u6309\u56de\u8f66\u952e\u7acb\u5373\u5f00\u5c40\uff0c\u7ee7\u7eed\u4f60\u7684\u4e0b\u4e00\u8f6e\u63a8\u8fdb",
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
        u8"\u6309\u52a0\u8f7d\u952e\u6f14\u793a\u5f02\u6b65\u52a0\u8f7d\uff0c\u5f55\u5c4f\u65f6\u53ef\u76f4\u63a5\u5c55\u793a\u4e0d\u5361\u987f\u7684\u52a0\u8f7d\u52a8\u753b",
        u8"\u5f02\u6b65\u52a0\u8f7d\u6f14\u793a",
        u8"\u540e\u53f0\u7ebf\u7a0b\u6b63\u5728\u51c6\u5907\u8d44\u6e90\uff0c\u4e3b\u7ebf\u7a0b\u7ee7\u7eed\u6e32\u67d3\uff0c\u4e0d\u4f1a\u5361\u4f4f\u754c\u9762",
        u8"\u52a0\u8f7d\u5b8c\u6210\uff0c\u7816\u5757\u4e3b\u9898\u5df2\u5237\u65b0",
        u8"\u6309\u52a0\u8f7d\u952e\u53ef\u518d\u6b21\u89e6\u53d1",
        u8"\u5f53\u524d\u8fdb\u5ea6",
        u8"\u8d44\u6e90\u70ed\u52a0\u8f7d",
        u8"\u52a0\u8f7d\u671f\u95f4\u4ecd\u53ef\u7ee7\u7eed\u6e32\u67d3\u548c\u64cd\u4f5c",
        u8"\u68c0\u6d4b\u5230\u4e0a\u6b21\u8fd0\u884c\u5b58\u6863",
        u8"\u6309\u7ee7\u7eed\u952e\u4ece\u5b58\u6863\u5173\u5361\u5f00\u59cb \u6309\u56de\u8f66\u952e\u65b0\u5f00\u4e00\u5c40 \u91cd\u65b0\u5f00\u5c40",
        u8"\u7ee7\u7eed\u5b58\u6863",
        u8"\u65b0\u5f00\u4e00\u5c40",
        u8"\u7f16\u8f91\u6a21\u5f0f",
        u8"\u6309 E \u9000\u51fa\u7f16\u8f91 \u5de6\u952e\u6dfb\u52a0 \u53f3\u952e\u5220\u9664 Ctrl+S \u4fdd\u5b58 1-6 \u5207\u5f62\u72b6 [ ] \u8c03\u8010\u4e45",
        u8"\u5df2\u4fdd\u5b58\u5f53\u524d\u5173\u5361\u5e03\u5c40",
        u8"\u5173\u5361 JSON \u7f3a\u5931\u6216\u683c\u5f0f\u9519\u8bef \u5df2\u56de\u9000\u5230\u9ed8\u8ba4\u5e03\u5c40",
        u8"\u5b58\u6863\u5df2\u4ece\u65e7\u7248\u683c\u5f0f\u8fc1\u79fb\u5230\u65b0\u7248",
        u8"\u5b58\u6863\u635f\u574f\u6216\u8bfb\u53d6\u5931\u8d25 \u5df2\u5ffd\u7565",
        u8"\u5f97\u5206 \u547d \u7403 \u8d2f \u8c03\u8bd5 \u664b\u7ea7\u6311\u6218 \u91d1\u5e01 \u91cd\u65b0\u5f00\u5c40 \u6311\u6218\u7ed3\u675f \u7ed3\u7b97\u603b\u5206 \u5168\u90e8\u901a\u5173",
        u8"\u5b9e\u65f6\u589e\u76ca \u8d2f\u7a7f\u5f3a\u51fb \u5e38\u9a7b \u6e38\u620f\u6682\u505c \u52a0\u8f7d\u4e2d \u8fdb\u5ea6 \u4e3b\u9898 \u7403\u4f53 \u6321\u677f \u7816\u5757",
        u8"\u8d77\u624b\u95e8 \u5bf9\u51b2\u6d41 \u87ba\u65cb\u57ce \u53cc\u7ffc\u5939\u51fb \u4e2d\u8def\u7206\u7834 \u7ec8\u5c40\u73af\u9635",
        u8"\u5ce1\u8c37\u7a7f\u884c \u9636\u68af\u51b2\u523a \u8702\u5de2\u88c2\u53d8 \u53cc\u5854\u56f4\u653b \u98ce\u8f66\u56de\u65cb \u7ec8\u6781\u6d2a\u6d41",
        u8"\u672c\u5173 \u5173\u5361\u7a81\u7834 \u4e0b\u4e00\u9635\u5217\u5df2\u5c55\u5f00 \u7b2c",
        u8"\u723d\u5feb\u77ed\u5c40 \u7acb\u5373\u5f00\u6253 \u5f00\u5c40 \u5b58\u6863 \u672c\u8f6e\u51c6\u5907 \u7b2c\u4e00\u79d2\u5c31\u5f00\u59cb\u51b2\u51fb",
        u8"\u8fde\u51fb \u500d\u7387",
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

Game::Game(
    int width,
    int height,
    bool autoStart,
    bool autoContinue,
    int autoExitLevel,
    float autoExitHoldSeconds,
    std::string frameCaptureDir)
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
    , bootAutoStart(autoStart)
    , bootAutoContinue(autoContinue)
    , demoExitLevel(autoExitLevel)
    , demoExitHoldSeconds(autoExitHoldSeconds)
    , demoExitElapsed(0.0F)
    , captureFrameDir(std::move(frameCaptureDir))
    , captureFramesEnabled(false)
    , captureFrameIndex(0)
    , captureFrameStride(4)
{
    if (!captureFrameDir.empty())
    {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::create_directories(this->captureFrameDir, ec);
        captureFramesEnabled = !ec;
    }

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
    LoadGameplayArt();
    LoadBackgroundTextures();
    currentLevel = 1;
    LoadLevelForCurrentStage();
    ResetBalls();
    const Rectangle initialPaddleRect = paddle.GetRect();
    prevPaddlePosition = Vector2 {initialPaddleRect.x, initialPaddleRect.y};
    RefreshResumeSaveState();
    if (bootAutoContinue && hasResumeSave)
    {
        (void)ContinueSavedRun();
    }
    else if (bootAutoStart)
    {
        StartNewRun();
    }
}

Game::~Game()
{
    JoinAsyncLoadThread();
    SaveRuntimeProgress();
    SavePlayerProfile();
    UnloadGameplayArt();
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

void Game::SaveRuntimeProgress()
{
    if (currentState != GameState::PLAYING && currentState != GameState::PAUSED)
    {
        return;
    }

    game::RunSaveState saveState;
    saveState.version = game::RunSaveState::CURRENT_VERSION;
    saveState.currentLevel = std::max(1, currentLevel);
    saveState.score = std::max(0, score);
    saveState.lives = std::max(1, lives);
    saveState.hasData = true;

    std::string errorMessage;
    if (!game::RunSaveStore::save(kRunSavePath, saveState, &errorMessage))
    {
        ShowNotice(u8"\u8fd0\u884c\u5b58\u6863\u4fdd\u5b58\u5931\u8d25", "Failed to save run progress", 4.0F);
        return;
    }

    hasResumeSave = true;
    resumeSaveState = saveState;
}

void Game::ClearRuntimeSave()
{
    std::string errorMessage;
    if (!game::RunSaveStore::remove(kRunSavePath, &errorMessage))
    {
        ShowNotice(u8"\u8fd0\u884c\u5b58\u6863\u5220\u9664\u5931\u8d25", "Failed to remove run save", 4.0F);
    }
    hasResumeSave = false;
    continuePromptVisible = false;
    resumeSaveState = game::RunSaveState {};
}

void Game::RefreshResumeSaveState()
{
    const game::RunSaveLoadResult loadResult = game::RunSaveStore::load(kRunSavePath);
    if (!loadResult.exists)
    {
        hasResumeSave = false;
        continuePromptVisible = false;
        resumeSaveState = game::RunSaveState {};
        return;
    }

    if (!loadResult.loaded)
    {
        hasResumeSave = false;
        continuePromptVisible = false;
        resumeSaveState = game::RunSaveState {};
        ShowNotice(
            u8"\u5b58\u6863\u635f\u574f\u6216\u8bfb\u53d6\u5931\u8d25 \u5df2\u5ffd\u7565",
            "Save file was invalid and has been ignored",
            5.2F);
        return;
    }

    resumeSaveState = loadResult.state;
    hasResumeSave = loadResult.state.hasData;
    continuePromptVisible = hasResumeSave;
    if (loadResult.migrated)
    {
        std::string errorMessage;
        (void)game::RunSaveStore::save(kRunSavePath, resumeSaveState, &errorMessage);
        ShowNotice(
            u8"\u5b58\u6863\u5df2\u4ece\u65e7\u7248\u683c\u5f0f\u8fc1\u79fb\u5230\u65b0\u7248",
            "Legacy save migrated to v2 format",
            5.0F);
    }
}

void Game::ShowNotice(const char* zhText, const char* enText, float seconds)
{
    noticeZh = zhText == nullptr ? "" : zhText;
    noticeEn = enText == nullptr ? "" : enText;
    noticeTimer = std::max(seconds, 0.0F);
}

void Game::RegisterComboHit(bool destroyed)
{
    if (!destroyed)
    {
        comboTimer = std::max(comboTimer, 1.25F);
        return;
    }

    ++comboCount;
    comboTimer = std::clamp(2.6F + (float)comboCount * 0.030F, 2.6F, 4.2F);
    comboPulseTimer = 0.32F;

    if (comboCount >= 22)
    {
        comboMultiplier = 5;
    }
    else if (comboCount >= 14)
    {
        comboMultiplier = 4;
    }
    else if (comboCount >= 8)
    {
        comboMultiplier = 3;
    }
    else if (comboCount >= 3)
    {
        comboMultiplier = 2;
    }
    else
    {
        comboMultiplier = 1;
    }
}

void Game::ResetCombo()
{
    comboCount = 0;
    comboMultiplier = 1;
    comboTimer = 0.0F;
    comboPulseTimer = 0.0F;
}

void Game::StartNewRun()
{
    currentState = GameState::PLAYING;
    victory = false;
    editorModeActive = false;
    demoExitElapsed = 0.0F;
    score = 0;
    currentLevel = 1;
    powerUpSystem.clear(world);
    edgeParticles.clear();
    particleSystem.clear();
    pendingPierceCharges = 0;
    hitFlashTimer = 0.0F;
    powerSurgeTimer = 0.0F;
    ResetCombo();
    lastObservedPowerUpCollections = playerProfile.totalPowerUpsCollected;
    ++playerProfile.totalRuns;

    ClearRuntimeSave();
    LoadLevelForCurrentStage();
    lives = levelStartLives;
    ResetBalls();
    const Rectangle resetPaddleRect = paddle.GetRect();
    prevPaddlePosition = Vector2 {resetPaddleRect.x, resetPaddleRect.y};
    SaveRuntimeProgress();
    SavePlayerProfile();
    writeUserPrefsFile(isDarkMode, ballColorIndex, paddleColorIndex, brickColorIndex);
}

bool Game::ContinueSavedRun()
{
    if (!hasResumeSave)
    {
        return false;
    }

    currentState = GameState::PLAYING;
    victory = false;
    editorModeActive = false;
    demoExitElapsed = 0.0F;
    score = std::max(0, resumeSaveState.score);
    currentLevel = std::clamp(resumeSaveState.currentLevel, 1, (int)game_style::levelConfigs().size());
    powerUpSystem.clear(world);
    edgeParticles.clear();
    particleSystem.clear();
    pendingPierceCharges = 0;
    hitFlashTimer = 0.0F;
    powerSurgeTimer = 0.0F;
    ResetCombo();
    lastObservedPowerUpCollections = playerProfile.totalPowerUpsCollected;

    LoadLevelForCurrentStage();
    lives = std::max(1, resumeSaveState.lives);
    ResetBalls();
    const Rectangle resetPaddleRect = paddle.GetRect();
    prevPaddlePosition = Vector2 {resetPaddleRect.x, resetPaddleRect.y};
    continuePromptVisible = false;
    SaveRuntimeProgress();
    return true;
}

bool Game::LoadLevelForCurrentStage()
{
    const auto& levels = game_style::levelConfigs();
    if (levels.empty())
    {
        return false;
    }

    const std::size_t levelIndex = (std::size_t)std::clamp(currentLevel - 1, 0, (int)levels.size() - 1);
    currentLevelPath = levels[levelIndex];
    InitConfigAndBricks(currentLevelPath);
    return true;
}

void Game::ToggleEditorMode()
{
    if (currentState != GameState::PLAYING)
    {
        return;
    }

    editorModeActive = !editorModeActive;
    if (editorModeActive)
    {
        ShowNotice(
            u8"\u7f16\u8f91\u6a21\u5f0f\u5df2\u5f00\u542f",
            "Edit mode enabled",
            3.0F);
    }
    else
    {
        ShowNotice(
            u8"\u7f16\u8f91\u6a21\u5f0f\u5df2\u5173\u95ed",
            "Edit mode disabled",
            2.8F);
    }
}

void Game::HandleEditorInput()
{
    if (!editorModeActive)
    {
        return;
    }

    if (IsKeyPressed(KEY_ONE)) editorShape = 0;
    if (IsKeyPressed(KEY_TWO)) editorShape = 1;
    if (IsKeyPressed(KEY_THREE)) editorShape = 2;
    if (IsKeyPressed(KEY_FOUR)) editorShape = 3;
    if (IsKeyPressed(KEY_FIVE)) editorShape = 4;
    if (IsKeyPressed(KEY_SIX)) editorShape = 5;
    if (IsKeyPressed(KEY_LEFT_BRACKET))
    {
        editorDurability = 1;
    }
    if (IsKeyPressed(KEY_RIGHT_BRACKET))
    {
        editorDurability = 1;
    }

    const bool savePressed = IsKeyPressed(KEY_S)
        && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
    if (savePressed)
    {
        SaveCurrentLevelLayout();
    }

    const Vector2 mousePos = GetMousePosition();
    const float snappedX = std::round((mousePos.x - 52.0F) / editorBrickWidth) * editorBrickWidth + 52.0F;
    const float snappedY = std::round((mousePos.y - 132.0F) / editorBrickHeight) * editorBrickHeight + 132.0F;
    const Rectangle snappedRect {snappedX, snappedY, editorBrickWidth - 8.0F, editorBrickHeight - 8.0F};

    auto hitIt = std::find_if(
        bricks.begin(),
        bricks.end(),
        [mousePos](const Brick& brick)
        {
            return brick.IsActive() && CheckCollisionPointRec(mousePos, brick.GetRect());
        });

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && hitIt != bricks.end())
    {
        const std::size_t index = (std::size_t)std::distance(bricks.begin(), hitIt);
        bricks.erase(hitIt);
        if (index < brickPowerUps.size())
        {
            brickPowerUps.erase(brickPowerUps.begin() + (long long)index);
        }
        RebuildBrickSpatialGrid();
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hitIt == bricks.end())
    {
        bricks.emplace_back(
            snappedRect.x,
            snappedRect.y,
            snappedRect.width,
            snappedRect.height,
            editorShape,
            1);
        brickPowerUps.push_back(std::nullopt);
        RebuildBrickSpatialGrid();
    }
}

void Game::SaveCurrentLevelLayout()
{
    game::LevelData snapshot = currentLevelData;
    snapshot.bricks.clear();
    for (const Brick& brick : bricks)
    {
        if (!brick.IsActive())
        {
            continue;
        }

        const Rectangle rect = brick.GetRect();
        snapshot.bricks.push_back(game::BrickRecord {
            rect.x,
            rect.y,
            rect.width,
            rect.height,
            brick.GetShape(),
            1,
        });
    }

    std::string errorMessage;
    if (levelLoader.saveToFile(currentLevelPath, snapshot, &errorMessage))
    {
        currentLevelData = snapshot;
        ShowNotice(
            u8"\u5df2\u4fdd\u5b58\u5f53\u524d\u5173\u5361\u5e03\u5c40",
            "Current level layout saved",
            3.4F);
    }
    else
    {
        ShowNotice(
            u8"\u5173\u5361 JSON \u4fdd\u5b58\u5931\u8d25",
            "Failed to save level JSON",
            4.4F);
    }
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
    const game::LevelData levelData = levelLoader.loadFromFile(levelJsonFile, currentLevel, screenWidth, screenHeight);
    ApplyLevelData(levelData);
    if (levelData.usedFallback)
    {
        ShowNotice(
            u8"\u5173\u5361 JSON \u7f3a\u5931\u6216\u683c\u5f0f\u9519\u8bef \u5df2\u56de\u9000\u5230\u9ed8\u8ba4\u5e03\u5c40",
            "Level JSON missing or invalid, fallback layout used",
            5.4F);
    }
}

void Game::ApplyLevelData(const game::LevelData& levelData)
{
    currentLevelData = levelData;
    bricks.clear();
    edgeParticles.clear();

    levelStartLives = std::max(1, levelData.lives);
    paddle = Paddle(
        levelData.paddleStart.x,
        levelData.paddleStart.y,
        levelData.paddleStart.width,
        levelData.paddleStart.height);
    effectPaddle = std::make_unique<game::Paddle>(game_style::toGameRect(levelData.paddleStart));
    world.paddle = effectPaddle.get();

    spawnBallPosition = levelData.ballStart;
    spawnBallVelocity = Vector2 {levelData.ballSpeed.x * 68.0F, levelData.ballSpeed.y * 68.0F};
    spawnBallRadius = std::clamp(levelData.ballRadius * 1.35F, 18.0F, 28.0F);
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

    std::vector<std::string> imagePaths = collectImagePaths("assets/art/backgrounds");
    if (imagePaths.empty())
    {
        imagePaths = collectImagePaths("image");
    }
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

    auto loadPack = [](const std::vector<BackgroundCandidate>& candidates, BackgroundPack& pack)
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

        pack.levels.reserve(candidates.size());
        for (const BackgroundCandidate& candidate : candidates)
        {
            const Texture2D levelTexture = LoadTexture(candidate.path.c_str());
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

void Game::LoadGameplayArt()
{
    UnloadGameplayArt();

    gameArt.ball = LoadTexture("assets/art/breakout/ball_blue.png");
    gameArt.paddle = LoadTexture("assets/art/breakout/paddle_capsule.png");

    const char* brickPaths[] {
        "assets/art/breakout/brick_blue.png",
        "assets/art/breakout/brick_green.png",
        "assets/art/breakout/brick_purple.png",
        "assets/art/breakout/brick_red.png",
        "assets/art/breakout/brick_orange.png",
        "assets/art/breakout/brick_cyan.png",
        "assets/art/breakout/brick_yellow.png",
        "assets/art/breakout/brick_deepgreen.png",
        "assets/art/breakout/brick_steel.png",
        "assets/art/breakout/brick_brown.png",
    };

    for (std::size_t index = 0; index < gameArt.bricks.size(); ++index)
    {
        gameArt.bricks[index] = LoadTexture(brickPaths[index]);
    }
}

void Game::UnloadGameplayArt()
{
    if (gameArt.ball.id != 0)
    {
        UnloadTexture(gameArt.ball);
        gameArt.ball = Texture2D {};
    }
    if (gameArt.paddle.id != 0)
    {
        UnloadTexture(gameArt.paddle);
        gameArt.paddle = Texture2D {};
    }
    for (Texture2D& texture : gameArt.bricks)
    {
        if (texture.id != 0)
        {
            UnloadTexture(texture);
            texture = Texture2D {};
        }
    }
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
    const std::size_t pick = levelIndex % pack.levels.size();
    return &pack.levels[pick];
}

const Texture2D* Game::ResolveBrickTexture(std::size_t brickIndex) const
{
    if (gameArt.bricks.empty())
    {
        return nullptr;
    }

    const std::size_t textureIndex = (brickIndex + (std::size_t)std::max(currentLevel - 1, 0)) % gameArt.bricks.size();
    const Texture2D& texture = gameArt.bricks[textureIndex];
    return texture.id != 0 ? &texture : nullptr;
}

Game::MenuStyleButtonRects Game::MenuStyleButtons() const
{
    const Rectangle shell {72.0F, (float)screenHeight * 0.19F, (float)screenWidth - 144.0F, (float)screenHeight * 0.56F};
    const Rectangle hero {shell.x + 24.0F, shell.y + 24.0F, shell.width * 0.62F, shell.height - 48.0F};
    const Rectangle side {hero.x + hero.width + 24.0F, shell.y + 24.0F, shell.width - hero.width - 72.0F, shell.height - 48.0F};

    const Rectangle buttonArea {side.x + 24.0F, side.y + side.height - 126.0F, side.width - 48.0F, 110.0F};
    const float gap = 12.0F;
    const float buttonWidth = (buttonArea.width - gap) * 0.5F;
    const float buttonHeight = 48.0F;

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
        SaveRuntimeProgress();
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

    if (IsKeyPressed(KEY_E) && currentState == GameState::PLAYING && !victory)
    {
        ToggleEditorMode();
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

        if (!asyncLoadActive && hasResumeSave && (IsKeyPressed(KEY_C)))
        {
            (void)ContinueSavedRun();
        }

        if (!asyncLoadActive && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_N)))
        {
            StartNewRun();
        }
    }
    else if (currentState == GameState::PLAYING)
    {
        if (editorModeActive)
        {
            HandleEditorInput();
            return;
        }

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
    if (noticeTimer > 0.0F)
    {
        noticeTimer = std::max(0.0F, noticeTimer - deltaSeconds);
    }
    if (impactFreezeTimer > 0.0F)
    {
        impactFreezeTimer = std::max(0.0F, impactFreezeTimer - deltaSeconds);
    }
    if (screenShakeTimer > 0.0F)
    {
        screenShakeTimer = std::max(0.0F, screenShakeTimer - deltaSeconds);
        if (screenShakeTimer <= 0.0F)
        {
            screenShakeMagnitude = 0.0F;
        }
    }
    if (hitFlashTimer > 0.0F)
    {
        hitFlashTimer = std::max(0.0F, hitFlashTimer - deltaSeconds);
    }
    if (powerSurgeTimer > 0.0F)
    {
        powerSurgeTimer = std::max(0.0F, powerSurgeTimer - deltaSeconds);
    }
    if (comboTimer > 0.0F)
    {
        comboTimer = std::max(0.0F, comboTimer - deltaSeconds);
        if (comboTimer <= 0.0F)
        {
            ResetCombo();
        }
    }
    if (comboPulseTimer > 0.0F)
    {
        comboPulseTimer = std::max(0.0F, comboPulseTimer - deltaSeconds);
    }
    if (levelClearTimer > 0.0F)
    {
        levelClearTimer = std::max(0.0F, levelClearTimer - deltaSeconds);
    }

    if (currentState != GameState::PLAYING || victory || editorModeActive)
    {
        UpdateEdgeParticles();
        return;
    }

    if (impactFreezeTimer > 0.0F)
    {
        particleSystem.update(deltaSeconds);
        UpdateEdgeParticles();
        return;
    }

    if (demoExitLevel > 0 && currentLevel >= demoExitLevel)
    {
        demoExitElapsed += deltaSeconds;
        if (demoExitElapsed >= std::max(demoExitHoldSeconds, 0.8F))
        {
            SaveRuntimeProgress();
            exitWindowRequest = true;
            UpdateEdgeParticles();
            return;
        }
    }
    else
    {
        demoExitElapsed = 0.0F;
    }

    profileSaveAccumulator += deltaSeconds;
    if (profileSaveAccumulator >= 15.0F)
    {
        playerProfile.totalPlaySeconds += (int)profileSaveAccumulator;
        profileSaveAccumulator = 0.0F;
        SavePlayerProfile();
        SaveRuntimeProgress();
    }

    const Rectangle paddleRect = paddle.GetRect();
    const game::Vec2 paddleVelocity {
        (paddleRect.x - prevPaddlePosition.x) / deltaSeconds,
        (paddleRect.y - prevPaddlePosition.y) / deltaSeconds,
    };
    SyncEffectPaddleToGameplay();
    HandleBalls(paddleVelocity, deltaSeconds);

    const int collectedBefore = playerProfile.totalPowerUpsCollected;
    powerUpSystem.update(deltaSeconds, world, (float)screenHeight);
    if (playerProfile.totalPowerUpsCollected > collectedBefore || playerProfile.totalPowerUpsCollected > lastObservedPowerUpCollections)
    {
        powerSurgeTimer = 0.72F;
        hitFlashTimer = std::max(hitFlashTimer, 0.18F);
        TriggerImpact(0.028F, 8.0F);
    }
    lastObservedPowerUpCollections = playerProfile.totalPowerUpsCollected;
    particleSystem.update(deltaSeconds);

    ApplyEffectPaddleToGameplay();
    UpdateEdgeParticles();
    const Rectangle updatedPaddleRect = paddle.GetRect();
    prevPaddlePosition = Vector2 {updatedPaddleRect.x, updatedPaddleRect.y};
}
