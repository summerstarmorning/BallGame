#include "LeaderboardSystem.hpp"
#include <algorithm>
#include <fstream>
#include "nlohmann_json.hpp"

using json = nlohmann::json;

namespace game
{

LeaderboardSystem::LeaderboardSystem()
{
}

void LeaderboardSystem::addEntry(const LeaderboardEntry& entry)
{
    localLeaderboard_.push_back(entry);
    
    // 按分数排序
    sortLeaderboard(localLeaderboard_, 0);
    
    // 保持最大条目数
    if (localLeaderboard_.size() > MAX_LOCAL_ENTRIES)
    {
        localLeaderboard_.resize(MAX_LOCAL_ENTRIES);
    }
}

void LeaderboardSystem::removeEntry(int index)
{
    if (index >= 0 && index < localLeaderboard_.size())
    {
        localLeaderboard_.erase(localLeaderboard_.begin() + index);
    }
}

void LeaderboardSystem::clear()
{
    localLeaderboard_.clear();
    globalLeaderboard_.clear();
}

int LeaderboardSystem::getLocalRank(int score) const
{
    int rank = 1;
    for (const auto& entry : localLeaderboard_)
    {
        if (entry.score > score)
        {
            rank++;
        }
        else
        {
            break;
        }
    }
    return rank;
}

int LeaderboardSystem::getGlobalRank(int score) const
{
    int rank = 1;
    for (const auto& entry : globalLeaderboard_)
    {
        if (entry.score > score)
        {
            rank++;
        }
        else
        {
            break;
        }
    }
    return rank;
}

std::vector<LeaderboardEntry> LeaderboardSystem::getTopScores(int count, bool global) const
{
    const auto& board = global ? globalLeaderboard_ : localLeaderboard_;
    std::vector<LeaderboardEntry> result;
    
    int limit = std::min(count, static_cast<int>(board.size()));
    result.insert(result.begin(), board.begin(), board.begin() + limit);
    
    return result;
}

std::vector<LeaderboardEntry> LeaderboardSystem::getTopByLevel(int count, bool global) const
{
    const auto& board = global ? globalLeaderboard_ : localLeaderboard_;
    auto sorted = board;
    
    std::sort(sorted.begin(), sorted.end(), 
        [](const LeaderboardEntry& a, const LeaderboardEntry& b)
        {
            return a.level > b.level;
        });
    
    std::vector<LeaderboardEntry> result;
    int limit = std::min(count, static_cast<int>(sorted.size()));
    result.insert(result.begin(), sorted.begin(), sorted.begin() + limit);
    
    return result;
}

std::vector<LeaderboardEntry> LeaderboardSystem::getTopBySurvivalTime(int count, bool global) const
{
    const auto& board = global ? globalLeaderboard_ : localLeaderboard_;
    auto sorted = board;
    
    std::sort(sorted.begin(), sorted.end(), 
        [](const LeaderboardEntry& a, const LeaderboardEntry& b)
        {
            return a.survivalTime > b.survivalTime;
        });
    
    std::vector<LeaderboardEntry> result;
    int limit = std::min(count, static_cast<int>(sorted.size()));
    result.insert(result.begin(), sorted.begin(), sorted.begin() + limit);
    
    return result;
}

void LeaderboardSystem::saveLocalLeaderboard(const std::string& path)
{
    json j = json::array();
    
    for (const auto& entry : localLeaderboard_)
    {
        json jEntry;
        jEntry["playerName"] = entry.playerName;
        jEntry["score"] = entry.score;
        jEntry["level"] = entry.level;
        jEntry["survivalTime"] = entry.survivalTime;
        jEntry["timestamp"] = entry.timestamp;
        jEntry["difficulty"] = entry.difficulty;
        jEntry["achievementCount"] = entry.achievementCount;
        j.push_back(jEntry);
    }
    
    std::ofstream out(path);
    if (out.is_open())
    {
        out << j.dump(4);
    }
}

void LeaderboardSystem::loadLocalLeaderboard(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open())
        return;
    
    try
    {
        json j;
        in >> j;
        
        localLeaderboard_.clear();
        for (const auto& jEntry : j)
        {
            LeaderboardEntry entry;
            entry.playerName = jEntry["playerName"];
            entry.score = jEntry["score"];
            entry.level = jEntry["level"];
            entry.survivalTime = jEntry["survivalTime"];
            entry.timestamp = jEntry["timestamp"];
            entry.difficulty = jEntry["difficulty"];
            entry.achievementCount = jEntry["achievementCount"];
            localLeaderboard_.push_back(entry);
        }
        
        sortLeaderboard(localLeaderboard_, 0);
    }
    catch (...)
    {
        // JSON解析错误
    }
}

void LeaderboardSystem::submitToGlobalLeaderboard(const LeaderboardEntry& entry)
{
    // TODO: 实现网络提交到全球排行榜
}

void LeaderboardSystem::fetchGlobalLeaderboard()
{
    // TODO: 实现从服务器获取全球排行榜
}

bool LeaderboardSystem::isNewPersonalBest(int score) const
{
    if (localLeaderboard_.empty())
        return true;
    
    return score > localLeaderboard_.front().score;
}

void LeaderboardSystem::sortLeaderboard(std::vector<LeaderboardEntry>& leaderboard, int sortBy)
{
    switch (sortBy)
    {
        case 0: // 按分数排序
            std::sort(leaderboard.begin(), leaderboard.end(),
                [](const LeaderboardEntry& a, const LeaderboardEntry& b)
                {
                    return a.score > b.score;
                });
            break;
        case 1: // 按关卡排序
            std::sort(leaderboard.begin(), leaderboard.end(),
                [](const LeaderboardEntry& a, const LeaderboardEntry& b)
                {
                    return a.level > b.level;
                });
            break;
        case 2: // 按生存时间排序
            std::sort(leaderboard.begin(), leaderboard.end(),
                [](const LeaderboardEntry& a, const LeaderboardEntry& b)
                {
                    return a.survivalTime > b.survivalTime;
                });
            break;
    }
}

} // namespace game
