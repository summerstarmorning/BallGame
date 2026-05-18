#include "Save/RunSaveState.hpp"

#include <filesystem>
#include <fstream>

#include "nlohmann_json.hpp"

using json = nlohmann::json;

namespace game
{
RunSaveLoadResult RunSaveStore::load(const std::string& path)
{
    RunSaveLoadResult result;

    std::ifstream input(path);
    if (!input.is_open())
    {
        return result;
    }

    result.exists = true;

    try
    {
        json document = json::parse(input, nullptr, true, true);

        int version = document.value("version", 1);
        if (version <= 1)
        {
            RunSaveState migrated;
            migrated.version = RunSaveState::CURRENT_VERSION;
            migrated.score = std::max(0, document.value("score", 0));
            migrated.lives = std::max(1, document.value("lives", 3));
            migrated.currentLevel = std::max(1, document.value("level", document.value("currentLevel", 1)));
            migrated.hasData = true;
            result.state = migrated;
            result.loaded = true;
            result.migrated = true;
            result.warning = "Migrated legacy save data to v2 format.";
            return result;
        }

        RunSaveState state;
        state.version = RunSaveState::CURRENT_VERSION;
        state.score = std::max(0, document.value("score", 0));
        state.lives = std::max(1, document.value("lives", 3));
        state.currentLevel = std::max(1, document.value("currentLevel", 1));
        state.hasData = true;

        result.state = state;
        result.loaded = true;
        return result;
    }
    catch (const std::exception& error)
    {
        result.warning = std::string("Save load failed: ") + error.what();
        return result;
    }
}

bool RunSaveStore::save(const std::string& path, const RunSaveState& state, std::string* errorMessage)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);

    json document;
    document["version"] = RunSaveState::CURRENT_VERSION;
    document["currentLevel"] = std::max(1, state.currentLevel);
    document["score"] = std::max(0, state.score);
    document["lives"] = std::max(1, state.lives);

    std::ofstream output(path);
    if (!output.is_open())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = "Failed to open save file for writing: " + path;
        }
        return false;
    }

    output << document.dump(4);
    return true;
}

bool RunSaveStore::remove(const std::string& path, std::string* errorMessage)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(path, ec))
    {
        return true;
    }

    if (!fs::remove(path, ec))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = "Failed to remove save file: " + path;
        }
        return false;
    }

    return true;
}
} // namespace game
