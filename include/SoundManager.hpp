#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "raylib.h"

namespace game
{

enum class SoundType
{
    BRICK_HIT,           // 击中砖块
    BALL_PADDLE_HIT,     // 球击中挡板
    WALL_BOUNCE,         // 墙壁反弹
    POWERUP_SPAWN,       // 道具生成
    POWERUP_COLLECT,     // 道具收集
    COMBO_MILESTONE,     // 连击里程碑（10、25、50、100等）
    LEVEL_COMPLETE,      // 关卡完成
    GAME_OVER,           // 游戏结束
    LIFE_LOST,           // 失去一条命
    MENU_SELECT,         // 菜单选择
    MENU_CONFIRM,        // 菜单确认
    BUTTON_CLICK,        // 按钮点击
    ACHIEVEMENT_UNLOCK,  // 成就解锁
    UI_HOVER,            // UI悬停
};

class SoundManager
{
public:
    SoundManager();
    ~SoundManager();
    
    void initialize();
    void shutdown();
    
    void playSound(SoundType soundType, float volume = 1.0f);
    void playSoundEx(SoundType soundType, float volume, float pitch, float pan = 0.5f);
    
    void setMasterVolume(float volume);
    void setSoundEffectsVolume(float volume);
    void setMusicVolume(float volume);
    
    float getMasterVolume() const noexcept { return masterVolume_; }
    float getSoundEffectsVolume() const noexcept { return sfxVolume_; }
    float getMusicVolume() const noexcept { return musicVolume_; }
    
    void update();
    
    bool isInitialized() const noexcept { return initialized_; }
    
private:
    struct SoundEntry
    {
        Sound sound;
        std::string filePath;
        bool loaded;
    };
    
    std::unordered_map<SoundType, SoundEntry> sounds_;
    float masterVolume_;
    float sfxVolume_;
    float musicVolume_;
    bool initialized_;
    
    void loadSound(SoundType soundType, const std::string& filePath);
    void unloadAllSounds();
    void loadDefaultSounds();
};

} // namespace game
