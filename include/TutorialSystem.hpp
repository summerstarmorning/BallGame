#pragma once

#include <string>
#include <vector>

namespace game
{

enum class TutorialStep
{
    MOVEMENT,           // 学习移动
    BALL_PHYSICS,       // 学习球的物理
    POWERUPS,           // 学习道具
    COMBOS,             // 学习连击
    DIFFICULTY,         // 学习难度选择
    ACHIEVEMENTS,       // 学习成就
    LEADERBOARD,        // 学习排行榜
    COMPLETE
};

struct TutorialContent
{
    TutorialStep step;
    std::string titleZh;
    std::string titleEn;
    std::string descriptionZh;
    std::string descriptionEn;
    std::string actionPromptZh;
    std::string actionPromptEn;
    bool completed;
};

class TutorialSystem
{
public:
    TutorialSystem();
    
    void initialize();
    void start();
    void skip();
    
    TutorialStep getCurrentStep() const noexcept { return currentStep_; }
    const TutorialContent& getCurrentContent() const { return tutorialSteps_[static_cast<int>(currentStep_)]; }
    
    void nextStep();
    void completeCurrentStep();
    
    bool isActive() const noexcept { return active_; }
    bool isCompleted() const noexcept { return currentStep_ == TutorialStep::COMPLETE; }
    float getProgress() const noexcept { return progress_; }
    
    void update(float deltaSeconds);
    void highlightArea(float x, float y, float width, float height);
    
private:
    std::vector<TutorialContent> tutorialSteps_;
    TutorialStep currentStep_;
    bool active_;
    float progress_;
    float stepTimer_;
    
    void initializeTutorialContent();
};

} // namespace game
