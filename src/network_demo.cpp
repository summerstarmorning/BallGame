#define SDL_MAIN_HANDLED

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Networking/NetworkSession.hpp"

namespace
{
constexpr int kScreenWidth = 1360;
constexpr int kScreenHeight = 820;
constexpr float kPaddleWidth = 156.0F;
constexpr float kPaddleHeight = 22.0F;
constexpr float kBallRadius = 12.0F;
constexpr float kPaddleSpeed = 560.0F;
constexpr float kSnapshotRateHz = 25.0F;
constexpr double kInterpolationDelaySeconds = 0.10;

enum class DemoMode
{
    Host,
    Client,
};

struct DemoOptions
{
    DemoMode mode {DemoMode::Host};
    std::string serverIp {"127.0.0.1"};
    std::uint16_t port {7777U};
    float simulatedLoss {0.0F};
};

struct ColorRgba
{
    Uint8 r {255U};
    Uint8 g {255U};
    Uint8 b {255U};
    Uint8 a {255U};
};

struct UiFonts
{
    TTF_Font* heading {nullptr};
    TTF_Font* body {nullptr};
    TTF_Font* mono {nullptr};

    void destroy()
    {
        if (heading != nullptr)
        {
            TTF_CloseFont(heading);
            heading = nullptr;
        }
        if (body != nullptr)
        {
            TTF_CloseFont(body);
            body = nullptr;
        }
        if (mono != nullptr)
        {
            TTF_CloseFont(mono);
            mono = nullptr;
        }
    }
};

struct DemoSimulation
{
    networking::GameState state {};

    void initialize()
    {
        state = networking::GameState {};
        state.playerCount = 2U;
        state.authoritativePlayerId = 0U;

        state.paddles[0].active = true;
        state.paddles[0].playerId = 0U;
        state.paddles[0].position = networking::Vec2f {(float)kScreenWidth * 0.5F, (float)kScreenHeight - 78.0F};

        state.paddles[1].active = true;
        state.paddles[1].playerId = 1U;
        state.paddles[1].position = networking::Vec2f {(float)kScreenWidth * 0.5F, 206.0F};

        resetBall(1.0F);
    }

    void applyHostInput(float moveDirection, float deltaSeconds)
    {
        movePaddle(0U, moveDirection, deltaSeconds);
    }

    void applyRemoteInput(const networking::PaddleInput& input, float deltaSeconds)
    {
        networking::PaddleState& remotePaddle = state.paddles[1];
        const float previousX = remotePaddle.position.x;
        remotePaddle.position.x = std::clamp(input.paddleCenterX, kPaddleWidth * 0.5F, (float)kScreenWidth - kPaddleWidth * 0.5F);
        remotePaddle.velocity.x = (remotePaddle.position.x - previousX) / std::max(deltaSeconds, 0.0001F);
        remotePaddle.velocity.y = 0.0F;
    }

    void update(float deltaSeconds)
    {
        deltaSeconds = std::min(deltaSeconds, 1.0F / 20.0F);
        ++state.tick;
        state.serverTimeSeconds += deltaSeconds;

        state.ball.position.x += state.ball.velocity.x * deltaSeconds;
        state.ball.position.y += state.ball.velocity.y * deltaSeconds;

        if (state.ball.position.x <= kBallRadius)
        {
            state.ball.position.x = kBallRadius;
            state.ball.velocity.x = std::abs(state.ball.velocity.x);
        }
        if (state.ball.position.x >= (float)kScreenWidth - kBallRadius)
        {
            state.ball.position.x = (float)kScreenWidth - kBallRadius;
            state.ball.velocity.x = -std::abs(state.ball.velocity.x);
        }

        handlePaddleCollision(0U, 1.0F);
        handlePaddleCollision(1U, -1.0F);

        if (state.ball.position.y > (float)kScreenHeight + kBallRadius)
        {
            ++state.scores[1];
            resetBall(-1.0F);
        }
        else if (state.ball.position.y < 188.0F - kBallRadius)
        {
            ++state.scores[0];
            resetBall(1.0F);
        }
    }

private:
    void movePaddle(std::size_t playerIndex, float moveDirection, float deltaSeconds)
    {
        networking::PaddleState& paddle = state.paddles[playerIndex];
        const float previousX = paddle.position.x;
        paddle.position.x += moveDirection * kPaddleSpeed * deltaSeconds;
        paddle.position.x = std::clamp(paddle.position.x, kPaddleWidth * 0.5F, (float)kScreenWidth - kPaddleWidth * 0.5F);
        paddle.velocity.x = (paddle.position.x - previousX) / std::max(deltaSeconds, 0.0001F);
        paddle.velocity.y = 0.0F;
    }

    void resetBall(float travelDirectionY)
    {
        state.ball.active = true;
        state.ball.position = networking::Vec2f {(float)kScreenWidth * 0.5F, (float)kScreenHeight * 0.56F};
        state.ball.velocity = networking::Vec2f {220.0F, 320.0F * travelDirectionY};
    }

    void handlePaddleCollision(std::size_t playerIndex, float yDirectionAfterBounce)
    {
        const networking::PaddleState& paddle = state.paddles[playerIndex];
        const float paddleLeft = paddle.position.x - kPaddleWidth * 0.5F;
        const float paddleRight = paddle.position.x + kPaddleWidth * 0.5F;
        const float paddleTop = paddle.position.y - kPaddleHeight * 0.5F;
        const float paddleBottom = paddle.position.y + kPaddleHeight * 0.5F;

        if (state.ball.position.x + kBallRadius < paddleLeft || state.ball.position.x - kBallRadius > paddleRight
            || state.ball.position.y + kBallRadius < paddleTop || state.ball.position.y - kBallRadius > paddleBottom)
        {
            return;
        }

        if ((playerIndex == 0U && state.ball.velocity.y > 0.0F) || (playerIndex == 1U && state.ball.velocity.y < 0.0F))
        {
            const float hitOffset = (state.ball.position.x - paddle.position.x) / (kPaddleWidth * 0.5F);
            state.ball.position.y = playerIndex == 0U ? paddleTop - kBallRadius : paddleBottom + kBallRadius;
            state.ball.velocity.y = std::abs(state.ball.velocity.y) * yDirectionAfterBounce;
            state.ball.velocity.x += hitOffset * 120.0F + paddle.velocity.x * 0.22F;
            state.ball.velocity.x = std::clamp(state.ball.velocity.x, -420.0F, 420.0F);
        }
    }
};

DemoOptions parseOptions(int argc, char** argv)
{
    DemoOptions options {};
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--mode" && index + 1 < argc)
        {
            const std::string mode = argv[++index];
            options.mode = mode == "client" ? DemoMode::Client : DemoMode::Host;
        }
        else if (argument == "--server-ip" && index + 1 < argc)
        {
            options.serverIp = argv[++index];
        }
        else if (argument == "--port" && index + 1 < argc)
        {
            options.port = (std::uint16_t)std::stoi(argv[++index]);
        }
        else if (argument == "--simulate-loss" && index + 1 < argc)
        {
            options.simulatedLoss = std::clamp(std::stof(argv[++index]), 0.0F, 0.95F);
        }
    }
    return options;
}

std::string resolveChineseFontPath()
{
    const std::array<const char*, 4> candidates {
        "C:/Windows/Fonts/msyhbd.ttc",
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "assets/fonts/Orbitron-Bold.ttf",
    };

    for (const char* candidate : candidates)
    {
        if (std::filesystem::exists(candidate))
        {
            return candidate;
        }
    }

    return {};
}

bool drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, float x, float y, ColorRgba color)
{
    if (renderer == nullptr || font == nullptr || text.empty())
    {
        return false;
    }

    const SDL_Color sdlColor {color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.size(), sdlColor);
    if (surface == nullptr)
    {
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr)
    {
        SDL_DestroySurface(surface);
        return false;
    }

    const SDL_FRect destination {x, y, (float)surface->w, (float)surface->h};
    SDL_RenderTexture(renderer, texture, nullptr, &destination);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    return true;
}

void fillRect(SDL_Renderer* renderer, const SDL_FRect& rectangle, ColorRgba color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rectangle);
}

void strokeRect(SDL_Renderer* renderer, const SDL_FRect& rectangle, ColorRgba color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(renderer, &rectangle);
}

void drawCircle(SDL_Renderer* renderer, float centerX, float centerY, float radius, ColorRgba color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const int minY = (int)std::floor(centerY - radius);
    const int maxY = (int)std::ceil(centerY + radius);
    for (int y = minY; y <= maxY; ++y)
    {
        const float offsetY = (float)y - centerY;
        const float delta = radius * radius - offsetY * offsetY;
        if (delta < 0.0F)
        {
            continue;
        }

        const float span = std::sqrt(delta);
        SDL_RenderLine(renderer, centerX - span, (float)y, centerX + span, (float)y);
    }
}

void drawGrid(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 26, 50, 78, 140);
    for (int x = 0; x < kScreenWidth; x += 36)
    {
        SDL_RenderLine(renderer, (float)x, 188.0F, (float)x, (float)kScreenHeight);
    }
    for (int y = 188; y < kScreenHeight; y += 36)
    {
        SDL_RenderLine(renderer, 0.0F, (float)y, (float)kScreenWidth, (float)y);
    }
}

void drawBackdrop(SDL_Renderer* renderer)
{
    for (int y = 0; y < kScreenHeight; ++y)
    {
        const float t = (float)y / (float)kScreenHeight;
        const Uint8 r = (Uint8)(10.0F + 8.0F * t);
        const Uint8 g = (Uint8)(16.0F + 18.0F * t);
        const Uint8 b = (Uint8)(28.0F + 34.0F * t);
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderLine(renderer, 0.0F, (float)y, (float)kScreenWidth, (float)y);
    }

    fillRect(renderer, SDL_FRect {0.0F, 0.0F, (float)kScreenWidth, 188.0F}, ColorRgba {7, 14, 26, 246});
    fillRect(renderer, SDL_FRect {22.0F, 18.0F, (float)kScreenWidth - 44.0F, 138.0F}, ColorRgba {15, 26, 44, 218});
    strokeRect(renderer, SDL_FRect {22.0F, 18.0F, (float)kScreenWidth - 44.0F, 138.0F}, ColorRgba {76, 160, 255, 110});
    fillRect(renderer, SDL_FRect {0.0F, 184.0F, (float)kScreenWidth, 4.0F}, ColorRgba {76, 160, 255, 170});
    drawGrid(renderer);
}

void drawPanel(SDL_Renderer* renderer, const SDL_FRect& panel, ColorRgba fill, ColorRgba border)
{
    fillRect(renderer, panel, fill);
    strokeRect(renderer, panel, border);
}

void drawPaddle(SDL_Renderer* renderer, const networking::PaddleState& paddle, ColorRgba color)
{
    if (!paddle.active)
    {
        return;
    }

    const SDL_FRect rectangle {
        paddle.position.x - kPaddleWidth * 0.5F,
        paddle.position.y - kPaddleHeight * 0.5F,
        kPaddleWidth,
        kPaddleHeight,
    };
    fillRect(renderer, rectangle, color);
    strokeRect(renderer, rectangle, ColorRgba {255, 255, 255, 70});
}

std::string hostTitle()
{
    return u8"\u4e3b\u673a / \u6743\u5a01\u670d\u52a1\u5668";
}

std::string clientTitle()
{
    return u8"\u5ba2\u6237\u7aef / \u63d2\u503c\u6e32\u67d3";
}

void drawScene(
    SDL_Renderer* renderer,
    const UiFonts& fonts,
    const networking::GameState& state,
    bool authoritativeView,
    const DemoOptions& options,
    const std::string& statusText)
{
    drawBackdrop(renderer);

    const SDL_FRect leftPanel {32.0F, 228.0F, 324.0F, 540.0F};
    const SDL_FRect rightPanel {(float)kScreenWidth - 356.0F, 228.0F, 324.0F, 540.0F};
    drawPanel(renderer, leftPanel, ColorRgba {13, 24, 40, 212}, ColorRgba {78, 157, 241, 110});
    drawPanel(renderer, rightPanel, ColorRgba {13, 24, 40, 212}, ColorRgba {78, 157, 241, 110});

    drawText(renderer, fonts.heading, authoritativeView ? hostTitle() : clientTitle(), 48.0F, 34.0F, ColorRgba {233, 241, 255, 255});
    drawText(renderer, fonts.body, statusText, 50.0F, 82.0F, ColorRgba {117, 213, 255, 255});

    char metrics[256] {};
    std::snprintf(
        metrics,
        sizeof(metrics),
        u8"\u672c\u673a\u6d4b\u8bd5\uff1a127.0.0.1:%u  \u4e22\u5305 %.0f%%  \u540c\u6b65 %.0f \u6b21/\u79d2",
        options.port,
        options.simulatedLoss * 100.0F,
        kSnapshotRateHz);
    drawText(renderer, fonts.body, metrics, 50.0F, 116.0F, ColorRgba {207, 223, 246, 255});

    drawText(
        renderer,
        fonts.body,
        authoritativeView ? std::string(u8"\u4e3b\u673a\u63a7\u5236\uff1aA / D\u3002\u4e3b\u673a\u8d1f\u8d23\u7403\u3001\u5206\u6570\u3001\u78b0\u649e\u4e0e\u5feb\u7167\u5e7f\u64ad\u3002")
                          : std::string(u8"\u5ba2\u6237\u7aef\u63a7\u5236\uff1a\u65b9\u5411\u952e\u5de6\u53f3\u3002\u5ba2\u6237\u7aef\u53ea\u53d1\u677f\u4f4d\u7f6e\u5e76\u6e32\u67d3\u5e73\u6ed1\u7ed3\u679c\u3002"),
        50.0F,
        144.0F,
        ColorRgba {255, 198, 110, 255});

    drawText(renderer, fonts.body, u8"\u7f51\u7edc\u6982\u89c8", leftPanel.x + 18.0F, leftPanel.y + 18.0F, ColorRgba {112, 202, 255, 255});
    drawText(
        renderer,
        fonts.body,
        authoritativeView ? std::string(u8"\u89d2\u8272\uff1a\u4e3b\u673a") : std::string(u8"\u89d2\u8272\uff1a\u5ba2\u6237\u7aef"),
        leftPanel.x + 18.0F,
        leftPanel.y + 54.0F,
        ColorRgba {235, 241, 253, 255});
    std::snprintf(metrics, sizeof(metrics), u8"Tick\uff1a%u", state.tick);
    drawText(renderer, fonts.body, metrics, leftPanel.x + 18.0F, leftPanel.y + 88.0F, ColorRgba {235, 241, 253, 255});
    std::snprintf(metrics, sizeof(metrics), u8"\u670d\u52a1\u5668\u65f6\u95f4\uff1a%.2f \u79d2", state.serverTimeSeconds);
    drawText(renderer, fonts.body, metrics, leftPanel.x + 18.0F, leftPanel.y + 122.0F, ColorRgba {235, 241, 253, 255});
    std::snprintf(metrics, sizeof(metrics), u8"\u63d2\u503c\u7f13\u51b2\uff1a%.0f ms", kInterpolationDelaySeconds * 1000.0);
    drawText(renderer, fonts.body, metrics, leftPanel.x + 18.0F, leftPanel.y + 156.0F, ColorRgba {235, 241, 253, 255});
    drawText(renderer, fonts.body, u8"\u540c\u6b65\u7ed3\u6784\uff1a\u7403\u4f4d\u7f6e / \u901f\u5ea6\u3001\u677f\u4f4d\u7f6e / \u901f\u5ea6\u3001\u5206\u6570", leftPanel.x + 18.0F, leftPanel.y + 210.0F, ColorRgba {185, 200, 228, 255});
    drawText(renderer, fonts.body, u8"\u5e8f\u5217\u5316\uff1a\u7ed3\u6784\u4f53 + memcpy + \u663e\u5f0f\u5b57\u8282\u5e8f\u7edf\u4e00", leftPanel.x + 18.0F, leftPanel.y + 244.0F, ColorRgba {185, 200, 228, 255});
    drawText(renderer, fonts.body, u8"ENetHost\uff1a\u672c\u5730\u7f51\u7edc\u7aef\u70b9", leftPanel.x + 18.0F, leftPanel.y + 298.0F, ColorRgba {185, 200, 228, 255});
    drawText(renderer, fonts.body, u8"ENetPeer\uff1a\u8fdc\u7aef\u8fde\u63a5", leftPanel.x + 18.0F, leftPanel.y + 332.0F, ColorRgba {185, 200, 228, 255});
    drawText(renderer, fonts.body, u8"ENetPacket\uff1a\u4f20\u8f93\u5305", leftPanel.x + 18.0F, leftPanel.y + 366.0F, ColorRgba {185, 200, 228, 255});

    drawText(renderer, fonts.body, u8"\u672c\u673a\u53cc\u5f00\u6307\u5f15", rightPanel.x + 18.0F, rightPanel.y + 18.0F, ColorRgba {112, 202, 255, 255});
    drawText(renderer, fonts.body, u8"1. \u5148\u542f\u52a8\u4e3b\u673a\u7a97\u53e3", rightPanel.x + 18.0F, rightPanel.y + 54.0F, ColorRgba {235, 241, 253, 255});
    drawText(renderer, fonts.body, u8"2. \u518d\u542f\u52a8\u5ba2\u6237\u7aef\u8fde\u63a5 127.0.0.1", rightPanel.x + 18.0F, rightPanel.y + 88.0F, ColorRgba {235, 241, 253, 255});
    drawText(renderer, fonts.body, u8"3. \u8c03\u9ad8\u4e22\u5305\u53c2\u6570\u67e5\u770b\u63d2\u503c\u6548\u679c", rightPanel.x + 18.0F, rightPanel.y + 122.0F, ColorRgba {235, 241, 253, 255});
    std::snprintf(metrics, sizeof(metrics), u8"\u4e3b\u673a\u5206\u6570\uff1a%d", state.scores[0]);
    drawText(renderer, fonts.heading, metrics, rightPanel.x + 18.0F, rightPanel.y + 180.0F, ColorRgba {111, 217, 255, 255});
    std::snprintf(metrics, sizeof(metrics), u8"\u5ba2\u6237\u7aef\u5206\u6570\uff1a%d", state.scores[1]);
    drawText(renderer, fonts.heading, metrics, rightPanel.x + 18.0F, rightPanel.y + 228.0F, ColorRgba {255, 138, 187, 255});
    drawText(renderer, fonts.body, u8"\u53ef\u6269\u5c55\uff1a\u591a\u4eba\u3001\u89c2\u6218\u3001\u4e3b\u673a\u8fc1\u79fb\u3001\u65ad\u7ebf\u91cd\u8fde", rightPanel.x + 18.0F, rightPanel.y + 300.0F, ColorRgba {185, 200, 228, 255});

    fillRect(renderer, SDL_FRect {382.0F, 228.0F, 596.0F, 540.0F}, ColorRgba {7, 16, 27, 120});
    strokeRect(renderer, SDL_FRect {382.0F, 228.0F, 596.0F, 540.0F}, ColorRgba {77, 148, 229, 84});

    drawCircle(renderer, state.ball.position.x, state.ball.position.y, kBallRadius + 8.0F, ColorRgba {255, 190, 89, 30});
    drawCircle(renderer, state.ball.position.x, state.ball.position.y, kBallRadius, ColorRgba {255, 193, 85, 255});
    drawPaddle(renderer, state.paddles[0], ColorRgba {93, 214, 255, 255});
    drawPaddle(renderer, state.paddles[1], ColorRgba {255, 116, 164, 255});
}

bool loadUiFonts(UiFonts& fonts, std::string& errorMessage)
{
    const std::string fontPath = resolveChineseFontPath();
    if (fontPath.empty())
    {
        errorMessage = "No Chinese font file found.";
        return false;
    }

    fonts.heading = TTF_OpenFont(fontPath.c_str(), 30.0F);
    fonts.body = TTF_OpenFont(fontPath.c_str(), 19.0F);
    fonts.mono = TTF_OpenFont(fontPath.c_str(), 16.0F);
    if (fonts.heading == nullptr || fonts.body == nullptr || fonts.mono == nullptr)
    {
        errorMessage = "Failed to load UI fonts.";
        return false;
    }

    return true;
}
} // namespace

int main(int argc, char** argv)
{
    const DemoOptions options = parseOptions(argc, argv);
    const networking::EnetRuntime runtime {};
    if (!runtime.isReady())
    {
        std::cerr << "ENet initialization failed.\n";
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL3 initialization failed.\n";
        return 1;
    }
    if (!TTF_Init())
    {
        std::cerr << "SDL3_ttf initialization failed.\n";
        SDL_Quit();
        return 1;
    }

    SDL_SetAppMetadata("BallGame Networking Demo", "1.0.0", "com.ballgame.networking");

    const std::string windowTitle = options.mode == DemoMode::Host
        ? std::string(u8"\u7f51\u7edc\u4e3b\u673a\u8c03\u8bd5\u9762\u677f")
        : std::string(u8"\u7f51\u7edc\u5ba2\u6237\u7aef\u8c03\u8bd5\u9762\u677f");
    SDL_Window* window = SDL_CreateWindow(windowTitle.c_str(), kScreenWidth, kScreenHeight, 0);
    if (window == nullptr)
    {
        std::cerr << "Window creation failed.\n";
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr)
    {
        std::cerr << "Renderer creation failed.\n";
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    UiFonts fonts {};
    std::string errorMessage {};
    if (!loadUiFonts(fonts, errorMessage))
    {
        std::cerr << errorMessage << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    std::cout << "Local dual-instance test:\n";
    std::cout << "  1. Host:   network_sync_demo --mode host --port " << options.port << " --simulate-loss " << options.simulatedLoss << "\n";
    std::cout << "  2. Client: network_sync_demo --mode client --server-ip 127.0.0.1 --port " << options.port
              << " --simulate-loss " << options.simulatedLoss << "\n";

    bool running = true;
    Uint64 previousCounter = SDL_GetPerformanceCounter();

    if (options.mode == DemoMode::Host)
    {
        networking::HostSession hostSession {};
        networking::HostConfig config {};
        config.listenPort = options.port;
        config.packetLossChance = options.simulatedLoss;

        if (!hostSession.start(config, errorMessage))
        {
            std::cerr << errorMessage << '\n';
            fonts.destroy();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return 1;
        }

        DemoSimulation simulation {};
        simulation.initialize();
        double snapshotAccumulator = 0.0;

        while (running)
        {
            SDL_Event event {};
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    running = false;
                }
            }

            const Uint64 counter = SDL_GetPerformanceCounter();
            const double elapsed = (double)(counter - previousCounter) / (double)SDL_GetPerformanceFrequency();
            previousCounter = counter;
            const float deltaSeconds = (float)std::max(elapsed, 0.0001);

            const bool moveLeft = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A];
            const bool moveRight = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D];
            const float moveDirection = (moveRight ? 1.0F : 0.0F) - (moveLeft ? 1.0F : 0.0F);

            hostSession.service(0U);
            simulation.applyHostInput(moveDirection, deltaSeconds);
            if (const std::optional<networking::PaddleInput> remoteInput = hostSession.consumeRemoteInput())
            {
                simulation.applyRemoteInput(*remoteInput, deltaSeconds);
            }
            simulation.update(deltaSeconds);

            snapshotAccumulator += deltaSeconds;
            const double snapshotInterval = 1.0 / kSnapshotRateHz;
            while (snapshotAccumulator >= snapshotInterval)
            {
                hostSession.broadcastState(simulation.state);
                snapshotAccumulator -= snapshotInterval;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            drawScene(
                renderer,
                fonts,
                simulation.state,
                true,
                options,
                hostSession.isClientConnected() ? std::string(u8"\u5ba2\u6237\u7aef\u5df2\u8fde\u63a5\uff0c\u4e3b\u673a\u6b63\u5728\u5e7f\u64ad\u6743\u5a01\u72b6\u6001\u3002")
                                              : std::string(u8"\u7b49\u5f85\u5ba2\u6237\u7aef\u8fde\u63a5\u4e2d\u3002"));
            SDL_RenderPresent(renderer);
        }
    }
    else
    {
        networking::ClientSession clientSession {};
        networking::ClientConfig config {};
        config.hostIp = options.serverIp;
        config.hostPort = options.port;
        config.packetLossChance = options.simulatedLoss;

        if (!clientSession.connect(config, errorMessage))
        {
            std::cerr << errorMessage << '\n';
            fonts.destroy();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return 1;
        }

        networking::SnapshotInterpolator interpolator {};
        networking::GameState renderState {};
        renderState.playerCount = 2U;
        renderState.paddles[0].active = true;
        renderState.paddles[0].position = networking::Vec2f {(float)kScreenWidth * 0.5F, (float)kScreenHeight - 78.0F};
        renderState.paddles[1].active = true;
        renderState.paddles[1].position = networking::Vec2f {(float)kScreenWidth * 0.5F, 206.0F};
        renderState.ball.position = networking::Vec2f {(float)kScreenWidth * 0.5F, (float)kScreenHeight * 0.56F};

        float desiredPaddleX = renderState.paddles[1].position.x;
        std::uint32_t inputSequence = 0U;

        while (running)
        {
            SDL_Event event {};
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    running = false;
                }
            }

            const Uint64 counter = SDL_GetPerformanceCounter();
            const double elapsed = (double)(counter - previousCounter) / (double)SDL_GetPerformanceFrequency();
            previousCounter = counter;
            const float deltaSeconds = (float)std::max(elapsed, 0.0001);

            clientSession.service(0U);
            if (const std::optional<networking::SnapshotEnvelope> snapshot = clientSession.consumeSnapshot())
            {
                interpolator.pushSnapshot(*snapshot);
            }

            const bool moveLeft = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LEFT];
            const bool moveRight = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RIGHT];
            const float moveDirection = (moveRight ? 1.0F : 0.0F) - (moveLeft ? 1.0F : 0.0F);
            desiredPaddleX += moveDirection * kPaddleSpeed * deltaSeconds;
            desiredPaddleX = std::clamp(desiredPaddleX, kPaddleWidth * 0.5F, (float)kScreenWidth - kPaddleWidth * 0.5F);

            networking::PaddleInput input {};
            input.sequence = ++inputSequence;
            input.playerId = 1U;
            input.paddleCenterX = desiredPaddleX;
            input.paddleVelocityX = moveDirection * kPaddleSpeed;
            clientSession.sendInput(input);

            const double renderTime = SDL_GetTicks() / 1000.0 - kInterpolationDelaySeconds;
            interpolator.sample(renderTime, renderState);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            drawScene(
                renderer,
                fonts,
                renderState,
                false,
                options,
                clientSession.connectionState() == networking::ConnectionState::Connected
                    ? std::string(u8"\u5df2\u8fde\u63a5\u4e3b\u673a\uff0c\u6b63\u5728\u8fdb\u884c\u63d2\u503c\u6e32\u67d3\u3002")
                    : std::string(u8"\u6b63\u5728\u8fde\u63a5\u4e3b\u673a\u3002"));
            SDL_RenderPresent(renderer);
        }
    }

    fonts.destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
