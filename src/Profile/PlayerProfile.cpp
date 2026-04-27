#include "Profile/PlayerProfile.hpp"

#include <chrono>
#include <ctime>
#include <fstream>

#include "nlohmann_json.hpp"

namespace game
{
namespace
{
using json = nlohmann::json;

std::string currentUtcTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t clock = std::chrono::system_clock::to_time_t(now);
    std::tm utcTime {};
#ifdef _WIN32
    gmtime_s(&utcTime, &clock);
#else
    gmtime_r(&clock, &utcTime);
#endif

    char buffer[32] {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    return buffer;
}
} // namespace

PlayerProfile PlayerProfileStore::load(const std::filesystem::path& path)
{
    PlayerProfile profile {};

    std::ifstream input(path);
    if (input.is_open())
    {
        try
        {
            const json document = json::parse(input, nullptr, true, true);
            profile.profileId = document.value("profileId", profile.profileId);
            profile.createdAtUtc = document.value("createdAtUtc", profile.createdAtUtc);
            profile.lastPlayedAtUtc = document.value("lastPlayedAtUtc", profile.lastPlayedAtUtc);
            profile.totalSessions = document.value("totalSessions", profile.totalSessions);
            profile.totalRuns = document.value("totalRuns", profile.totalRuns);
            profile.totalPlaySeconds = document.value("totalPlaySeconds", profile.totalPlaySeconds);
            profile.bestScore = document.value("bestScore", profile.bestScore);
            profile.lifetimeScore = document.value("lifetimeScore", profile.lifetimeScore);
            profile.highestStage = document.value("highestStage", profile.highestStage);
            profile.softCurrency = document.value("softCurrency", profile.softCurrency);
            profile.totalBricksDestroyed = document.value("totalBricksDestroyed", profile.totalBricksDestroyed);
            profile.totalPowerUpsCollected = document.value("totalPowerUpsCollected", profile.totalPowerUpsCollected);
            profile.totalBallLosses = document.value("totalBallLosses", profile.totalBallLosses);
        }
        catch (const std::exception&)
        {
        }
    }

    const std::string nowUtc = currentUtcTimestamp();
    if (profile.createdAtUtc.empty())
    {
        profile.createdAtUtc = nowUtc;
    }
    profile.lastPlayedAtUtc = nowUtc;
    return profile;
}

bool PlayerProfileStore::save(const std::filesystem::path& path, const PlayerProfile& profile)
{
    json document;
    document["profileId"] = profile.profileId;
    document["createdAtUtc"] = profile.createdAtUtc;
    document["lastPlayedAtUtc"] = profile.lastPlayedAtUtc;
    document["totalSessions"] = profile.totalSessions;
    document["totalRuns"] = profile.totalRuns;
    document["totalPlaySeconds"] = profile.totalPlaySeconds;
    document["bestScore"] = profile.bestScore;
    document["lifetimeScore"] = profile.lifetimeScore;
    document["highestStage"] = profile.highestStage;
    document["softCurrency"] = profile.softCurrency;
    document["totalBricksDestroyed"] = profile.totalBricksDestroyed;
    document["totalPowerUpsCollected"] = profile.totalPowerUpsCollected;
    document["totalBallLosses"] = profile.totalBallLosses;

    std::ofstream output(path);
    if (!output.is_open())
    {
        return false;
    }

    output << document.dump(4);
    return true;
}
} // namespace game
