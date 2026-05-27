#pragma once

#include "GameTypes.hpp"
#include <vector>
#include <string>

namespace game
{

enum class BossPhase
{
    PHASE_1,
    PHASE_2,
    PHASE_3,
    DEFEATED
};

struct BossAttackPattern
{
    std::string patternName;
    float patternDuration;
    float projectileSpeed;
    int projectileCount;
    float projectileSpread;
};

class BossEnemy
{
public:
    BossEnemy(const std::string& name, int maxHealth);
    
    void initialize(Vec2 position);
    void update(float deltaSeconds);
    void takeDamage(int damage);
    
    const std::string& getName() const noexcept { return name_; }
    int getHealth() const noexcept { return health_; }
    int getMaxHealth() const noexcept { return maxHealth_; }
    float getHealthPercent() const noexcept { return health_ > 0 ? (health_ / static_cast<float>(maxHealth_)) : 0.0f; }
    
    Vec2 getPosition() const noexcept { return position_; }
    float getSize() const noexcept { return size_; }
    
    BossPhase getCurrentPhase() const noexcept { return currentPhase_; }
    bool isDefeated() const noexcept { return currentPhase_ == BossPhase::DEFEATED; }
    
    const std::vector<BossAttackPattern>& getAttackPatterns() const { return attackPatterns_; }
    const BossAttackPattern& getCurrentAttackPattern() const { return attackPatterns_[currentAttackIndex_]; }
    
    bool isAttacking() const noexcept { return phaseTimeElapsed_ < attackPatterns_[currentAttackIndex_].patternDuration; }
    float getAttackProgress() const;
    
private:
    std::string name_;
    int health_;
    int maxHealth_;
    Vec2 position_;
    float size_;
    
    BossPhase currentPhase_;
    float phaseHealth_;
    float phaseTimeElapsed_;
    
    std::vector<BossAttackPattern> attackPatterns_;
    int currentAttackIndex_;
    
    void updatePhase();
    void nextAttackPattern();
    void initializePhaseAttacks();
};

} // namespace game
