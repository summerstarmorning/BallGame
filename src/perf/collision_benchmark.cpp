#include "Performance/BrickSpatialGrid.hpp"
#include "nlohmann_json.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "raylib.h"

using json = nlohmann::json;

namespace
{
struct BenchmarkBall
{
    Vector2 position {};
    Vector2 velocity {};
    float radius {18.0F};
};

struct ScenarioConfig
{
    std::string name;
    int columns {0};
    int rows {0};
    int layers {1};
    int ballCount {1};
    int frames {0};
};

struct ScenarioData
{
    Rectangle worldBounds {};
    std::vector<Rectangle> bricks;
    std::vector<BenchmarkBall> balls;
};

struct RunStats
{
    double totalMilliseconds {0.0};
    double averageFrameMilliseconds {0.0};
    double equivalentFps {0.0};
    std::uint64_t checksum {0U};
};

ScenarioData buildScenario(const ScenarioConfig& config)
{
    ScenarioData data;
    constexpr float leftPadding = 48.0F;
    constexpr float topPadding = 126.0F;
    constexpr float brickWidth = 84.0F;
    constexpr float brickHeight = 26.0F;
    constexpr float horizontalGap = 10.0F;
    constexpr float verticalGap = 14.0F;
    constexpr float layerGap = 26.0F;

    const float width = leftPadding * 2.0F + (float)config.columns * brickWidth + (float)(config.columns - 1) * horizontalGap;
    const float height = topPadding * 2.0F + (float)config.rows * config.layers * (brickHeight + verticalGap) + (float)(config.layers - 1) * layerGap + 260.0F;
    data.worldBounds = Rectangle {0.0F, 0.0F, width, height};

    data.bricks.reserve((std::size_t)config.columns * (std::size_t)config.rows * (std::size_t)config.layers);
    for (int layer = 0; layer < config.layers; ++layer)
    {
        const float layerYOffset = (float)layer * ((float)config.rows * (brickHeight + verticalGap) + layerGap);
        for (int row = 0; row < config.rows; ++row)
        {
            const float rowYOffset = topPadding + layerYOffset + (float)row * (brickHeight + verticalGap);
            const float rowShift = (row % 2 == 0) ? 0.0F : brickWidth * 0.35F;
            for (int column = 0; column < config.columns; ++column)
            {
                const float x = leftPadding + rowShift + (float)column * (brickWidth + horizontalGap);
                data.bricks.push_back(Rectangle {x, rowYOffset, brickWidth, brickHeight});
            }
        }
    }

    data.balls.reserve((std::size_t)config.ballCount);
    const float spawnY = data.worldBounds.height - 132.0F;
    for (int index = 0; index < config.ballCount; ++index)
    {
        const float t = config.ballCount > 1 ? (float)index / (float)(config.ballCount - 1) : 0.5F;
        BenchmarkBall ball;
        ball.position = Vector2 {
            72.0F + t * std::max(120.0F, data.worldBounds.width - 144.0F),
            spawnY - (float)(index % 3) * 42.0F,
        };
        ball.velocity = Vector2 {
            340.0F + (float)((index * 37) % 140),
            -420.0F - (float)((index * 29) % 160),
        };
        ball.radius = 16.0F + (float)(index % 3);
        data.balls.push_back(ball);
    }

    return data;
}

void advanceBalls(std::vector<BenchmarkBall>& balls, const Rectangle& bounds, float deltaSeconds)
{
    for (BenchmarkBall& ball : balls)
    {
        ball.position.x += ball.velocity.x * deltaSeconds;
        ball.position.y += ball.velocity.y * deltaSeconds;

        if (ball.position.x - ball.radius < bounds.x)
        {
            ball.position.x = bounds.x + ball.radius;
            ball.velocity.x = std::abs(ball.velocity.x);
        }
        else if (ball.position.x + ball.radius > bounds.x + bounds.width)
        {
            ball.position.x = bounds.x + bounds.width - ball.radius;
            ball.velocity.x = -std::abs(ball.velocity.x);
        }

        if (ball.position.y - ball.radius < bounds.y + 32.0F)
        {
            ball.position.y = bounds.y + 32.0F + ball.radius;
            ball.velocity.y = std::abs(ball.velocity.y);
        }
        else if (ball.position.y + ball.radius > bounds.y + bounds.height - 32.0F)
        {
            ball.position.y = bounds.y + bounds.height - 32.0F - ball.radius;
            ball.velocity.y = -std::abs(ball.velocity.y);
        }
    }
}

RunStats runNaiveScenario(const ScenarioConfig& config)
{
    ScenarioData data = buildScenario(config);
    constexpr float deltaSeconds = 1.0F / 60.0F;

    RunStats stats;
    const auto started = std::chrono::steady_clock::now();
    for (int frame = 0; frame < config.frames; ++frame)
    {
        advanceBalls(data.balls, data.worldBounds, deltaSeconds);
        for (const BenchmarkBall& ball : data.balls)
        {
            for (std::size_t brickIndex = 0; brickIndex < data.bricks.size(); ++brickIndex)
            {
                if (!CheckCollisionCircleRec(ball.position, ball.radius, data.bricks[brickIndex]))
                {
                    continue;
                }

                stats.checksum += (std::uint64_t)brickIndex + 1U;
                break;
            }
        }
    }

    const auto finished = std::chrono::steady_clock::now();
    stats.totalMilliseconds = std::chrono::duration<double, std::milli>(finished - started).count();
    stats.averageFrameMilliseconds = stats.totalMilliseconds / (double)config.frames;
    stats.equivalentFps = stats.averageFrameMilliseconds > 0.0 ? 1000.0 / stats.averageFrameMilliseconds : 0.0;
    return stats;
}

RunStats runGridScenario(const ScenarioConfig& config)
{
    ScenarioData data = buildScenario(config);
    constexpr float deltaSeconds = 1.0F / 60.0F;

    game::BrickSpatialGrid grid;
    grid.rebuild(data.bricks, data.worldBounds, 112.0F, 52.0F);
    std::vector<std::size_t> candidates;
    candidates.reserve(32U);

    RunStats stats;
    const auto started = std::chrono::steady_clock::now();
    for (int frame = 0; frame < config.frames; ++frame)
    {
        advanceBalls(data.balls, data.worldBounds, deltaSeconds);
        for (const BenchmarkBall& ball : data.balls)
        {
            grid.queryCircle(ball.position, ball.radius, candidates);
            for (std::size_t brickIndex : candidates)
            {
                if (!CheckCollisionCircleRec(ball.position, ball.radius, data.bricks[brickIndex]))
                {
                    continue;
                }

                stats.checksum += (std::uint64_t)brickIndex + 1U;
                break;
            }
        }
    }

    const auto finished = std::chrono::steady_clock::now();
    stats.totalMilliseconds = std::chrono::duration<double, std::milli>(finished - started).count();
    stats.averageFrameMilliseconds = stats.totalMilliseconds / (double)config.frames;
    stats.equivalentFps = stats.averageFrameMilliseconds > 0.0 ? 1000.0 / stats.averageFrameMilliseconds : 0.0;
    return stats;
}

json makeScenarioResult(const ScenarioConfig& config)
{
    const RunStats naive = runNaiveScenario(config);
    const RunStats grid = runGridScenario(config);

    json result;
    result["name"] = config.name;
    result["brick_count"] = config.columns * config.rows * config.layers;
    result["ball_count"] = config.ballCount;
    result["frames"] = config.frames;
    result["naive"] = {
        {"total_ms", naive.totalMilliseconds},
        {"avg_frame_ms", naive.averageFrameMilliseconds},
        {"equivalent_fps", naive.equivalentFps},
        {"checksum", naive.checksum},
    };
    result["spatial_grid"] = {
        {"total_ms", grid.totalMilliseconds},
        {"avg_frame_ms", grid.averageFrameMilliseconds},
        {"equivalent_fps", grid.equivalentFps},
        {"checksum", grid.checksum},
    };
    result["checksums_match"] = naive.checksum == grid.checksum;
    result["speedup_ratio"] = grid.totalMilliseconds > 0.0 ? naive.totalMilliseconds / grid.totalMilliseconds : 0.0;
    result["fps_gain_percent"] = naive.equivalentFps > 0.0
        ? ((grid.equivalentFps - naive.equivalentFps) / naive.equivalentFps) * 100.0
        : 0.0;
    return result;
}
} // namespace

int main(int argc, char** argv)
{
    std::string outputPath = "docs/performance_benchmark_results.json";
    for (int index = 1; index < argc; ++index)
    {
        const std::string arg = argv[index];
        if (arg == "--output" && index + 1 < argc)
        {
            outputPath = argv[++index];
        }
    }

    const std::vector<ScenarioConfig> scenarios {
        {"standard_single_ball", 14, 7, 1, 1, 20000},
        {"dense_multiball", 16, 10, 3, 8, 12000},
        {"extreme_multiball", 18, 12, 4, 14, 8000},
    };

    json report;
    report["benchmark"] = "ballgame_brick_collision";
    report["timestamp_hint"] = "Generated locally from the same executable for before/after algorithm comparison";
    report["results"] = json::array();

    bool allChecksumsMatch = true;
    for (const ScenarioConfig& config : scenarios)
    {
        json result = makeScenarioResult(config);
        allChecksumsMatch = allChecksumsMatch && result.value("checksums_match", false);
        report["results"].push_back(std::move(result));
    }
    report["all_checksums_match"] = allChecksumsMatch;

    std::ofstream output(outputPath);
    if (!output.is_open())
    {
        std::cerr << "Failed to open benchmark output path: " << outputPath << '\n';
        return 1;
    }

    output << std::setw(2) << report << '\n';
    std::cout << std::setw(2) << report << '\n';
    return allChecksumsMatch ? 0 : 2;
}
