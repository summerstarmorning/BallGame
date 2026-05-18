#pragma once

#include <string>

namespace game
{
struct RunSaveState
{
    static constexpr int CURRENT_VERSION = 2;

    int version {CURRENT_VERSION};
    int currentLevel {1};
    int score {0};
    int lives {3};
    bool hasData {false};
};

struct RunSaveLoadResult
{
    RunSaveState state {};
    bool exists {false};
    bool loaded {false};
    bool migrated {false};
    std::string warning;
};

class RunSaveStore
{
public:
    static RunSaveLoadResult load(const std::string& path);
    static bool save(const std::string& path, const RunSaveState& state, std::string* errorMessage = nullptr);
    static bool remove(const std::string& path, std::string* errorMessage = nullptr);
};
} // namespace game
