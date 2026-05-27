#pragma once

#include <string>

namespace game
{

class ComboSystem
{
public:
    ComboSystem();
    
    void resetCombo();
    void addHit();
    void onMiss();
    
    int getCurrentCombo() const noexcept { return currentCombo_; }
    int getMaxCombo() const noexcept { return maxCombo_; }
    int getLastComboPoints() const noexcept { return lastComboPoints_; }
    
    int getComboMultiplier() const noexcept { return comboMultiplier_; }
    int calculateBonusScore(int baseScore) const;
    
    bool isComboActive() const noexcept { return currentCombo_ > 0; }
    float getComboTimeLeft() const noexcept { return comboTimeLeft_; }
    
    void update(float deltaSeconds);
    
    std::string getComboText() const;
    
private:
    int currentCombo_;
    int maxCombo_;
    int lastComboPoints_;
    int comboMultiplier_;
    
    static constexpr float COMBO_TIMEOUT = 2.5f;
    static constexpr int COMBO_THRESHOLD_FOR_2X = 5;
    static constexpr int COMBO_THRESHOLD_FOR_3X = 15;
    static constexpr int COMBO_THRESHOLD_FOR_5X = 30;
    static constexpr int COMBO_THRESHOLD_FOR_10X = 50;
    
    float comboTimeLeft_;
};

} // namespace game
