#pragma once

#include <filesystem>
#include <string>

namespace game
{
struct PlayerProfile
{
    std::string profileId {"local"};
    std::string createdAtUtc {};
    std::string lastPlayedAtUtc {};
    int totalSessions {0};
    int totalRuns {0};
    int totalPlaySeconds {0};
    int bestScore {0};
    int lifetimeScore {0};
    int highestStage {1};
    int softCurrency {0};
    int totalBricksDestroyed {0};
    int totalPowerUpsCollected {0};
    int totalBallLosses {0};
};

class PlayerProfileStore
{
public:
    static PlayerProfile load(const std::filesystem::path& path);
    static bool save(const std::filesystem::path& path, const PlayerProfile& profile);
};
} // namespace game
