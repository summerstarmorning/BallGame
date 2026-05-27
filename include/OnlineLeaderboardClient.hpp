#pragma once

#include "LeaderboardSystem.hpp"
#include <future>
#include <string>
#include <vector>

namespace game
{

enum class NetworkStatus
{
    OFFLINE,
    CONNECTING,
    CONNECTED,
    ERROR
};

class OnlineLeaderboardClient
{
public:
    OnlineLeaderboardClient();
    ~OnlineLeaderboardClient();
    
    void initialize(const std::string& serverUrl);
    void shutdown();
    
    void submitScore(const LeaderboardEntry& entry);
    void fetchGlobalLeaderboard();
    void fetchPlayerRank(const std::string& playerName, int score);
    
    NetworkStatus getStatus() const noexcept { return status_; }
    const std::vector<LeaderboardEntry>& getGlobalLeaderboard() const noexcept { return globalLeaderboard_; }
    
    bool isSyncing() const noexcept { return isSyncing_; }
    float getSyncProgress() const noexcept { return syncProgress_; }
    
    std::string getLastErrorMessage() const noexcept { return lastErrorMessage_; }
    
private:
    std::string serverUrl_;
    NetworkStatus status_;
    bool isSyncing_;
    float syncProgress_;
    std::string lastErrorMessage_;
    
    std::vector<LeaderboardEntry> globalLeaderboard_;
    std::future<void> networkThread_;
    
    void connectToServer();
    void submitScoreAsync(const LeaderboardEntry& entry);
    void fetchLeaderboardAsync();
};

} // namespace game
