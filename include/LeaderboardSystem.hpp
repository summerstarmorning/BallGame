#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace game
{

struct LeaderboardEntry
{
    std::string playerName;
    int score;
    int level;
    float survivalTime;
    uint64_t timestamp;
    std::string difficulty;
    int achievementCount;
};

class LeaderboardSystem
{
public:
    LeaderboardSystem();
    
    void addEntry(const LeaderboardEntry& entry);
    void removeEntry(int index);
    void clear();
    
    const std::vector<LeaderboardEntry>& getLocalLeaderboard() const noexcept { return localLeaderboard_; }
    const std::vector<LeaderboardEntry>& getGlobalLeaderboard() const noexcept { return globalLeaderboard_; }
    
    int getLocalRank(int score) const;
    int getGlobalRank(int score) const;
    
    std::vector<LeaderboardEntry> getTopScores(int count, bool global = false) const;
    std::vector<LeaderboardEntry> getTopByLevel(int count, bool global = false) const;
    std::vector<LeaderboardEntry> getTopBySurvivalTime(int count, bool global = false) const;
    
    void saveLocalLeaderboard(const std::string& path);
    void loadLocalLeaderboard(const std::string& path);
    
    void submitToGlobalLeaderboard(const LeaderboardEntry& entry);
    void fetchGlobalLeaderboard();
    
    bool isNewPersonalBest(int score) const;
    
private:
    std::vector<LeaderboardEntry> localLeaderboard_;
    std::vector<LeaderboardEntry> globalLeaderboard_;
    
    static constexpr int MAX_LOCAL_ENTRIES = 100;
    static constexpr int MAX_DISPLAY_ENTRIES = 20;
    
    void sortLeaderboard(std::vector<LeaderboardEntry>& leaderboard, 
                        int sortBy = 0); // 0=score, 1=level, 2=survival
};

} // namespace game
