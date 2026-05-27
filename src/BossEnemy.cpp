#include "BossEnemy.hpp"
#include <algorithm>
#include <cmath>

namespace game
{

BossEnemy::BossEnemy(const std::string& name, int maxHealth)
    : name_(name), health_(maxHealth), maxHealth_(maxHealth), 
      position_({0.0f, 0.0f}), size_(80.0f),
      currentPhase_(BossPhase::PHASE_1), phaseHealth_(static_cast<float>(maxHealth)),
      phaseTimeElapsed_(0.0f), currentAttackIndex_(0)
{
    initializePhaseAttacks();
}

void BossEnemy::initializePhaseAttacks()
{
    attackPatterns_.clear();
    
    BossAttackPattern pattern1;
    pattern1.patternName = u8"单射";
    pattern1.patternDuration = 2.0f;
    pattern1.projectileSpeed = 200.0f;
    pattern1.projectileCount = 1;
    pattern1.projectileSpread = 0.0f;
    attackPatterns_.push_back(pattern1);
    
    BossAttackPattern pattern2;
    pattern2.patternName = u8"三射";
    pattern2.patternDuration = 2.5f;
    pattern2.projectileSpeed = 220.0f;
    pattern2.projectileCount = 3;
    pattern2.projectileSpread = 30.0f;
    attackPatterns_.push_back(pattern2);
    
    BossAttackPattern pattern3;
    pattern3.patternName = u8"扫射";
    pattern3.patternDuration = 3.0f;
    pattern3.projectileSpeed = 180.0f;
    pattern3.projectileCount = 5;
    pattern3.projectileSpread = 60.0f;
    attackPatterns_.push_back(pattern3);
}

void BossEnemy::initialize(Vec2 position)
{
    position_ = position;
    health_ = maxHealth_;
    phaseHealth_ = static_cast<float>(maxHealth_);
    currentPhase_ = BossPhase::PHASE_1;
    phaseTimeElapsed_ = 0.0f;
    currentAttackIndex_ = 0;
}

void BossEnemy::update(float deltaSeconds)
{
    phaseTimeElapsed_ += deltaSeconds;
    
    // 攻击模式轮换
    if (currentAttackIndex_ < attackPatterns_.size())
    {
        if (phaseTimeElapsed_ > attackPatterns_[currentAttackIndex_].patternDuration)
        {
            nextAttackPattern();
        }
    }
    
    updatePhase();
}

void BossEnemy::takeDamage(int damage)
{
    health_ -= damage;
    phaseHealth_ -= damage;
    
    if (health_ < 0)
        health_ = 0;
    
    updatePhase();
}

void BossEnemy::updatePhase()
{
    float healthPercent = getHealthPercent();
    
    if (healthPercent <= 0.0f)
    {
        currentPhase_ = BossPhase::DEFEATED;
    }
    else if (healthPercent <= 0.33f)
    {
        currentPhase_ = BossPhase::PHASE_3;
    }
    else if (healthPercent <= 0.66f)
    {
        currentPhase_ = BossPhase::PHASE_2;
    }
    else
    {
        currentPhase_ = BossPhase::PHASE_1;
    }
}

void BossEnemy::nextAttackPattern()
{
    currentAttackIndex_ = (currentAttackIndex_ + 1) % attackPatterns_.size();
    phaseTimeElapsed_ = 0.0f;
}

float BossEnemy::getAttackProgress() const
{
    if (currentAttackIndex_ >= attackPatterns_.size())
        return 0.0f;
    
    float duration = attackPatterns_[currentAttackIndex_].patternDuration;
    if (duration <= 0.0f)
        return 0.0f;
    
    return std::min(1.0f, phaseTimeElapsed_ / duration);
}

} // namespace game
