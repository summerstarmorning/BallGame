#include "ComboSystem.hpp"
#include <cmath>

namespace game
{

ComboSystem::ComboSystem() 
    : currentCombo_(0), maxCombo_(0), lastComboPoints_(0), comboMultiplier_(1), comboTimeLeft_(0.0f)
{
}

void ComboSystem::resetCombo()
{
    currentCombo_ = 0;
    comboMultiplier_ = 1;
    comboTimeLeft_ = 0.0f;
}

void ComboSystem::addHit()
{
    currentCombo_++;
    if (currentCombo_ > maxCombo_)
    {
        maxCombo_ = currentCombo_;
    }
    
    // 根据连击数更新倍数
    if (currentCombo_ >= COMBO_THRESHOLD_FOR_10X)
    {
        comboMultiplier_ = 10;
    }
    else if (currentCombo_ >= COMBO_THRESHOLD_FOR_5X)
    {
        comboMultiplier_ = 5;
    }
    else if (currentCombo_ >= COMBO_THRESHOLD_FOR_3X)
    {
        comboMultiplier_ = 3;
    }
    else if (currentCombo_ >= COMBO_THRESHOLD_FOR_2X)
    {
        comboMultiplier_ = 2;
    }
    else
    {
        comboMultiplier_ = 1;
    }
    
    // 重置超时计时器
    comboTimeLeft_ = COMBO_TIMEOUT;
}

void ComboSystem::onMiss()
{
    if (currentCombo_ > 0)
    {
        resetCombo();
    }
}

int ComboSystem::calculateBonusScore(int baseScore) const
{
    if (comboMultiplier_ <= 1)
        return baseScore;
    
    int bonusScore = baseScore * (comboMultiplier_ - 1);
    return bonusScore;
}

void ComboSystem::update(float deltaSeconds)
{
    if (currentCombo_ > 0)
    {
        comboTimeLeft_ -= deltaSeconds;
        if (comboTimeLeft_ <= 0.0f)
        {
            resetCombo();
        }
    }
}

std::string ComboSystem::getComboText() const
{
    if (currentCombo_ < 2)
        return "";
    
    if (comboMultiplier_ >= 10)
        return u8"🔥🔥🔥 超级连击！ x" + std::to_string(comboMultiplier_) + 
               u8" (" + std::to_string(currentCombo_) + u8")";
    else if (comboMultiplier_ >= 5)
        return u8"🔥🔥 杀戮连击 x" + std::to_string(comboMultiplier_) + 
               u8" (" + std::to_string(currentCombo_) + u8")";
    else if (comboMultiplier_ >= 3)
        return u8"🔥 不错! x" + std::to_string(comboMultiplier_) + 
               u8" (" + std::to_string(currentCombo_) + u8")";
    else if (comboMultiplier_ >= 2)
        return u8"连击 x" + std::to_string(comboMultiplier_) + 
               u8" (" + std::to_string(currentCombo_) + u8")";
    
    return "";
}

} // namespace game
