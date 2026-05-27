#include "SoundManager.hpp"
#include <iostream>
#include <algorithm>

namespace game
{

SoundManager::SoundManager() 
    : masterVolume_(1.0f), sfxVolume_(1.0f), musicVolume_(1.0f), initialized_(false)
{
}

SoundManager::~SoundManager()
{
    if (initialized_)
        shutdown();
}

void SoundManager::initialize()
{
    if (initialized_) return;
    
    InitAudioDevice();
    initialized_ = true;
    
    loadDefaultSounds();
}

void SoundManager::shutdown()
{
    unloadAllSounds();
    CloseAudioDevice();
    initialized_ = false;
}

void SoundManager::loadDefaultSounds()
{
    // 这里可以根据需要加载默认声音
    // 实际应用中应该有真实的音频文件路径
    // 现在我们只初始化映射表，实际音频文件可选
}

void SoundManager::loadSound(SoundType soundType, const std::string& filePath)
{
    // 实现实际的音频加载逻辑
    // 这取决于是否有真实的音频资源文件
}

void SoundManager::unloadAllSounds()
{
    for (auto& entry : sounds_)
    {
        if (entry.second.loaded)
        {
            UnloadSound(entry.second.sound);
        }
    }
    sounds_.clear();
}

void SoundManager::playSound(SoundType soundType, float volume)
{
    playSoundEx(soundType, volume, 1.0f, 0.5f);
}

void SoundManager::playSoundEx(SoundType soundType, float volume, float pitch, float pan)
{
    if (!initialized_) return;
    
    float effectiveVolume = masterVolume_ * sfxVolume_ * volume;
    effectiveVolume = std::max(0.0f, std::min(1.0f, effectiveVolume));
    
    auto it = sounds_.find(soundType);
    if (it != sounds_.end() && it->second.loaded)
    {
        SetSoundVolume(it->second.sound, effectiveVolume);
        PlaySound(it->second.sound);
    }
}

void SoundManager::setMasterVolume(float volume)
{
    masterVolume_ = std::max(0.0f, std::min(1.0f, volume));
}

void SoundManager::setSoundEffectsVolume(float volume)
{
    sfxVolume_ = std::max(0.0f, std::min(1.0f, volume));
}

void SoundManager::setMusicVolume(float volume)
{
    musicVolume_ = std::max(0.0f, std::min(1.0f, volume));
}

void SoundManager::update()
{
    // 音效系统的实时更新逻辑
    // 可以处理淡出、渐进等效果
}

} // namespace game
