#include "TutorialSystem.hpp"

namespace game
{

TutorialSystem::TutorialSystem() 
    : currentStep_(TutorialStep::MOVEMENT), active_(false), 
      progress_(0.0f), stepTimer_(0.0f)
{
    tutorialSteps_.resize(static_cast<int>(TutorialStep::COMPLETE) + 1);
    initializeTutorialContent();
}

void TutorialSystem::initializeTutorialContent()
{
    // MOVEMENT
    tutorialSteps_[0].step = TutorialStep::MOVEMENT;
    tutorialSteps_[0].titleZh = u8"移动挡板";
    tutorialSteps_[0].titleEn = "Move the Paddle";
    tutorialSteps_[0].descriptionZh = u8"使用左右方向键或A/D键移动挡板。";
    tutorialSteps_[0].descriptionEn = "Use arrow keys or A/D to move the paddle.";
    tutorialSteps_[0].actionPromptZh = u8"按←/→或A/D移动";
    tutorialSteps_[0].actionPromptEn = "Press ← → or A/D to move";
    tutorialSteps_[0].completed = false;
    
    // BALL_PHYSICS
    tutorialSteps_[1].step = TutorialStep::BALL_PHYSICS;
    tutorialSteps_[1].titleZh = u8"小球物理";
    tutorialSteps_[1].titleEn = "Ball Physics";
    tutorialSteps_[1].descriptionZh = u8"小球会从挡板反弹。尝试击中不同的位置来改变弹射角度。";
    tutorialSteps_[1].descriptionEn = "The ball bounces off the paddle. Try hitting different areas to change the angle.";
    tutorialSteps_[1].actionPromptZh = u8"击中10块砖块继续";
    tutorialSteps_[1].actionPromptEn = "Destroy 10 bricks to continue";
    tutorialSteps_[1].completed = false;
    
    // POWERUPS
    tutorialSteps_[2].step = TutorialStep::POWERUPS;
    tutorialSteps_[2].titleZh = u8"道具系统";
    tutorialSteps_[2].titleEn = "Power-ups";
    tutorialSteps_[2].descriptionZh = u8"击碎砖块时可能掉落道具。收集它们来获得特殊能力。";
    tutorialSteps_[2].descriptionEn = "Power-ups drop from bricks. Collect them for special abilities.";
    tutorialSteps_[2].actionPromptZh = u8"收集3个道具继续";
    tutorialSteps_[2].actionPromptEn = "Collect 3 power-ups to continue";
    tutorialSteps_[2].completed = false;
    
    // COMBOS
    tutorialSteps_[3].step = TutorialStep::COMBOS;
    tutorialSteps_[3].titleZh = u8"连击系统";
    tutorialSteps_[3].titleEn = "Combo System";
    tutorialSteps_[3].descriptionZh = u8"连续击中砖块会产生连击。连击数越多，分数倍数越高。";
    tutorialSteps_[3].descriptionEn = "Chain hits to build combos. Higher combos multiply your score.";
    tutorialSteps_[3].actionPromptZh = u8"达成5连击继续";
    tutorialSteps_[3].actionPromptEn = "Achieve 5 combo to continue";
    tutorialSteps_[3].completed = false;
    
    // DIFFICULTY
    tutorialSteps_[4].step = TutorialStep::DIFFICULTY;
    tutorialSteps_[4].titleZh = u8"难度选择";
    tutorialSteps_[4].titleEn = "Difficulty Modes";
    tutorialSteps_[4].descriptionZh = u8"选择合适的难度：简单、普通、困难、极难。不同难度改变游戏参数。";
    tutorialSteps_[4].descriptionEn = "Choose difficulty: Easy, Normal, Hard, Extreme. Each affects game parameters.";
    tutorialSteps_[4].actionPromptZh = u8"选择一个难度继续";
    tutorialSteps_[4].actionPromptEn = "Select a difficulty to continue";
    tutorialSteps_[4].completed = false;
    
    // ACHIEVEMENTS
    tutorialSteps_[5].step = TutorialStep::ACHIEVEMENTS;
    tutorialSteps_[5].titleZh = u8"成就系统";
    tutorialSteps_[5].titleEn = "Achievements";
    tutorialSteps_[5].descriptionZh = u8"完成目标来解锁成就。每个成就奖励游戏币。";
    tutorialSteps_[5].descriptionEn = "Unlock achievements by completing goals. Earn soft currency rewards.";
    tutorialSteps_[5].actionPromptZh = u8"打开成就面板";
    tutorialSteps_[5].actionPromptEn = "Open achievements panel";
    tutorialSteps_[5].completed = false;
    
    // LEADERBOARD
    tutorialSteps_[6].step = TutorialStep::LEADERBOARD;
    tutorialSteps_[6].titleZh = u8"排行榜";
    tutorialSteps_[6].titleEn = "Leaderboard";
    tutorialSteps_[6].descriptionZh = u8"与其他玩家竞争。查看本地和全球最高分。";
    tutorialSteps_[6].descriptionEn = "Compete with other players. View local and global top scores.";
    tutorialSteps_[6].actionPromptZh = u8"查看排行榜";
    tutorialSteps_[6].actionPromptEn = "View leaderboard";
    tutorialSteps_[6].completed = false;
    
    // COMPLETE
    tutorialSteps_[7].step = TutorialStep::COMPLETE;
    tutorialSteps_[7].titleZh = u8"教程完成！";
    tutorialSteps_[7].titleEn = "Tutorial Complete!";
    tutorialSteps_[7].descriptionZh = u8"你已学会所有基础知识。现在去享受游戏吧！";
    tutorialSteps_[7].descriptionEn = "You've learned the basics. Now go enjoy the game!";
    tutorialSteps_[7].actionPromptZh = u8"开始游戏";
    tutorialSteps_[7].actionPromptEn = "Start Game";
    tutorialSteps_[7].completed = false;
}

void TutorialSystem::initialize()
{
    currentStep_ = TutorialStep::MOVEMENT;
    progress_ = 0.0f;
    active_ = true;
}

void TutorialSystem::start()
{
    initialize();
}

void TutorialSystem::skip()
{
    active_ = false;
    currentStep_ = TutorialStep::COMPLETE;
}

void TutorialSystem::nextStep()
{
    if (currentStep_ < TutorialStep::COMPLETE)
    {
        currentStep_ = static_cast<TutorialStep>(static_cast<int>(currentStep_) + 1);
        stepTimer_ = 0.0f;
        
        if (currentStep_ == TutorialStep::COMPLETE)
        {
            active_ = false;
        }
    }
}

void TutorialSystem::completeCurrentStep()
{
    tutorialSteps_[static_cast<int>(currentStep_)].completed = true;
    nextStep();
}

void TutorialSystem::update(float deltaSeconds)
{
    if (!active_) return;
    
    stepTimer_ += deltaSeconds;
    
    int totalSteps = static_cast<int>(TutorialStep::COMPLETE);
    int currentStepIndex = static_cast<int>(currentStep_);
    progress_ = currentStepIndex / static_cast<float>(totalSteps);
}

void TutorialSystem::highlightArea(float x, float y, float width, float height)
{
    // 用于在屏幕上高亮特定区域的实现
    // 通常在Draw函数中使用
}

} // namespace game
