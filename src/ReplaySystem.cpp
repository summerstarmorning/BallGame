#include "ReplaySystem.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "nlohmann_json.hpp"

using json = nlohmann::json;

namespace game
{

ReplaySystem::ReplaySystem()
    : isRecording_(false), isPlaying_(false), playbackProgress_(0.0f),
      playbackSpeed_(1.0f), currentFrameIndex_(0), frameTimer_(0.0f)
{
}

ReplaySystem::~ReplaySystem()
{
    if (isRecording_)
        stopRecording();
}

void ReplaySystem::startRecording()
{
    isRecording_ = true;
    currentRecording_.clear();
}

void ReplaySystem::stopRecording()
{
    isRecording_ = false;
    // 将记录的帧数据转移到replayData_
    replayData_.frames = currentRecording_;
    replayData_.totalDuration = 
        currentRecording_.empty() ? 0.0f : 
        currentRecording_.back().timestamp;
}

void ReplaySystem::recordFrame(const ReplayFrame& frame)
{
    if (isRecording_)
    {
        currentRecording_.push_back(frame);
    }
}

void ReplaySystem::loadReplay(const std::string& filePath)
{
    loadReplayFromFile(filePath);
}

void ReplaySystem::startPlayback()
{
    if (replayData_.frames.empty())
        return;
    
    isPlaying_ = true;
    playbackProgress_ = 0.0f;
    currentFrameIndex_ = 0;
    frameTimer_ = 0.0f;
}

void ReplaySystem::stopPlayback()
{
    isPlaying_ = false;
    playbackProgress_ = 0.0f;
    currentFrameIndex_ = 0;
    frameTimer_ = 0.0f;
}

void ReplaySystem::update(float deltaSeconds)
{
    if (!isPlaying_ || replayData_.frames.empty())
        return;
    
    frameTimer_ += deltaSeconds * playbackSpeed_;
    
    // 查找当前帧
    while (currentFrameIndex_ < replayData_.frames.size() - 1 &&
           frameTimer_ > replayData_.frames[currentFrameIndex_ + 1].timestamp)
    {
        currentFrameIndex_++;
    }
    
    playbackProgress_ = (currentFrameIndex_ / static_cast<float>(replayData_.frames.size()));
    
    // 检查是否播放完毕
    if (currentFrameIndex_ >= replayData_.frames.size() - 1)
    {
        isPlaying_ = false;
        playbackProgress_ = 1.0f;
    }
}

void ReplaySystem::setPlaybackSpeed(float speed)
{
    playbackSpeed_ = std::max(0.1f, std::min(2.0f, speed));
}

const ReplayFrame& ReplaySystem::getCurrentFrame() const
{
    static const ReplayFrame emptyFrame = {};
    
    if (replayData_.frames.empty())
        return emptyFrame;
    
    if (currentFrameIndex_ >= replayData_.frames.size())
        return replayData_.frames.back();
    
    return replayData_.frames[currentFrameIndex_];
}

void ReplaySystem::saveReplay(const std::string& filePath)
{
    json jReplay;
    jReplay["playerName"] = replayData_.playerName;
    jReplay["finalScore"] = replayData_.finalScore;
    jReplay["finalLevel"] = replayData_.finalLevel;
    jReplay["totalDuration"] = replayData_.totalDuration;
    jReplay["difficulty"] = replayData_.difficulty;
    jReplay["recordedAt"] = replayData_.recordedAtTimestamp;
    
    json jFrames = json::array();
    for (const auto& frame : replayData_.frames)
    {
        json jFrame;
        jFrame["timestamp"] = frame.timestamp;
        jFrame["paddlePos"] = {frame.paddlePosition.x, frame.paddlePosition.y};
        jFrame["ballPos"] = {frame.ballPosition.x, frame.ballPosition.y};
        jFrame["ballVel"] = {frame.ballVelocity.x, frame.ballVelocity.y};
        jFrame["score"] = frame.score;
        jFrame["lives"] = frame.lives;
        jFrame["combo"] = frame.combo;
        jFrames.push_back(jFrame);
    }
    jReplay["frames"] = jFrames;
    
    std::ofstream out(filePath);
    if (out.is_open())
    {
        out << jReplay.dump(2);
    }
}

void ReplaySystem::loadReplayFromFile(const std::string& filePath)
{
    std::ifstream in(filePath);
    if (!in.is_open())
        return;
    
    try
    {
        json jReplay;
        in >> jReplay;
        
        replayData_.playerName = jReplay["playerName"];
        replayData_.finalScore = jReplay["finalScore"];
        replayData_.finalLevel = jReplay["finalLevel"];
        replayData_.totalDuration = jReplay["totalDuration"];
        replayData_.difficulty = jReplay["difficulty"];
        replayData_.recordedAtTimestamp = jReplay["recordedAt"];
        
        replayData_.frames.clear();
        for (const auto& jFrame : jReplay["frames"])
        {
            ReplayFrame frame;
            frame.timestamp = jFrame["timestamp"];
            auto padPos = jFrame["paddlePos"];
            frame.paddlePosition = {padPos[0], padPos[1]};
            auto ballPos = jFrame["ballPos"];
            frame.ballPosition = {ballPos[0], ballPos[1]};
            auto ballVel = jFrame["ballVel"];
            frame.ballVelocity = {ballVel[0], ballVel[1]};
            frame.score = jFrame["score"];
            frame.lives = jFrame["lives"];
            frame.combo = jFrame["combo"];
            replayData_.frames.push_back(frame);
        }
    }
    catch (const std::exception& e)
    {
        // JSON解析错误
    }
}

std::vector<std::string> ReplaySystem::getAvailableReplays() const
{
    std::vector<std::string> replays;
    
    try
    {
        std::string replayDir = "replays";
        for (const auto& entry : std::filesystem::directory_iterator(replayDir))
        {
            if (entry.path().extension() == ".replay")
            {
                replays.push_back(entry.path().filename().string());
            }
        }
    }
    catch (...)
    {
        // 目录不存在或无法访问
    }
    
    return replays;
}

} // namespace game
