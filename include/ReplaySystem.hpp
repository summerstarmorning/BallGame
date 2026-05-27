#pragma once

#include "GameTypes.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace game
{

struct ReplayFrame
{
    float timestamp;
    Vec2 paddlePosition;
    Vec2 ballPosition;
    Vec2 ballVelocity;
    int score;
    int lives;
    int combo;
};

struct ReplayData
{
    std::string playerName;
    int finalScore;
    int finalLevel;
    float totalDuration;
    std::string difficulty;
    uint64_t recordedAtTimestamp;
    std::vector<ReplayFrame> frames;
};

class ReplaySystem
{
public:
    ReplaySystem();
    ~ReplaySystem();
    
    // 记录
    void startRecording();
    void stopRecording();
    void recordFrame(const ReplayFrame& frame);
    bool isRecording() const noexcept { return isRecording_; }
    
    // 回放
    void loadReplay(const std::string& filePath);
    void startPlayback();
    void stopPlayback();
    void update(float deltaSeconds);
    
    bool isPlaying() const noexcept { return isPlaying_; }
    float getPlaybackProgress() const noexcept { return playbackProgress_; }
    float getPlaybackSpeed() const noexcept { return playbackSpeed_; }
    void setPlaybackSpeed(float speed);
    
    const ReplayFrame& getCurrentFrame() const;
    const ReplayData& getReplayData() const noexcept { return replayData_; }
    
    // 保存和加载
    void saveReplay(const std::string& filePath);
    void loadReplayFromFile(const std::string& filePath);
    
    // 回放列表
    std::vector<std::string> getAvailableReplays() const;
    
private:
    ReplayData replayData_;
    std::vector<ReplayFrame> currentRecording_;
    
    bool isRecording_;
    bool isPlaying_;
    float playbackProgress_;
    float playbackSpeed_;
    int currentFrameIndex_;
    float frameTimer_;
    
    void loadReplaysFromDisk();
};

} // namespace game
