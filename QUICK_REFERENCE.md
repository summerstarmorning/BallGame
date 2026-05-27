# 快速参考卡片

## 🚀 5分钟快速开始

### 1. 在Game.h中添加
```cpp
#include "GameEnhancementManager.hpp"

class Game {
private:
    game::GameEnhancementManager enhancementManager_;
};
```

### 2. 初始化（构造函数）
```cpp
Game::Game(...) {
    enhancementManager_.initialize();
}
```

### 3. 更新（Update函数）
```cpp
void Game::Update() {
    enhancementManager_.update(GetFrameTime());
}
```

### 4. 处理事件
```cpp
// 击碎砖块
enhancementManager_.onBrickDestroyed();

// 球击中挡板
enhancementManager_.onBallHit();

// 关卡完成
enhancementManager_.onLevelComplete(currentLevel, totalLevels, perfect);
```

---

## 📋 系统速查表

### 难度系统
```cpp
auto& diff = enhancementManager_.getDifficultyManager();
diff.setDifficulty(Difficulty::HARD);
float ballSpeedFactor = diff.getCurrentSettings().ballSpeedMultiplier;
```

### 连击系统
```cpp
auto& combo = enhancementManager_.getComboSystem();
combo.addHit();
int multiplier = combo.getComboMultiplier();  // 1, 2, 3, 5, 10
int bonus = combo.calculateBonusScore(100);
combo.update(deltaSeconds);
```

### 成就系统
```cpp
auto& ach = enhancementManager_.getAchievementSystem();
ach.onBrickDestroyed(totalBricksDestroyed);
ach.onScoreChanged(currentScore);
ach.onComboChanged(currentCombo);
int unlockedCount = ach.getUnlockedCount();
```

### 统计系统
```cpp
auto& stats = enhancementManager_.getStatisticsTracker();
stats.recordBrickDestroyed();
stats.recordHit();
stats.recordScore(1000);
float accuracy = stats.getStatistics().getAccuracy();
```

### 音效系统
```cpp
auto& sound = enhancementManager_.getSoundManager();
sound.playSound(SoundType::BRICK_HIT);
sound.setMasterVolume(0.8f);
```

### 排行榜系统
```cpp
auto& leaderboard = enhancementManager_.getLeaderboardSystem();
LeaderboardEntry entry;
entry.playerName = "Player";
entry.score = 50000;
leaderboard.addEntry(entry);
leaderboard.saveLocalLeaderboard("leaderboard.json");
```

### 教程系统
```cpp
auto& tutorial = enhancementManager_.getTutorialSystem();
if (tutorial.isActive()) {
    // 显示提示
    if (userCompletedAction) {
        tutorial.completeCurrentStep();
    }
}
```

### Boss系统
```cpp
game::BossEnemy boss("Boss Name", 500);
boss.initialize({800, 200});
boss.update(deltaSeconds);
if (ballHitsBoss) {
    boss.takeDamage(1);
}
```

---

## 🎯 常用事件处理

### 游戏开始
```cpp
enhancementManager_.onGameStart();
```

### 游戏结束
```cpp
enhancementManager_.onGameEnd(finalScore, survivalTime);
```

### 砖块击碎
```cpp
enhancementManager_.onBrickDestroyed();
```

### 球击中挡板
```cpp
enhancementManager_.onBallHit();
```

### 连击里程碑
```cpp
if (combo.getCurrentCombo() % 10 == 0) {
    enhancementManager_.onComboMilestone(combo.getCurrentCombo());
}
```

### 关卡完成
```cpp
bool perfect = (lives == levelStartLives);
enhancementManager_.onLevelComplete(level, totalLevels, perfect);
```

---

## 📊 数据访问速查

### 获取当前难度倍数
```cpp
const auto& settings = enhancementManager_.getDifficultyManager().getCurrentSettings();
float ballSpeedMult = settings.ballSpeedMultiplier;
float paddleSpeedMult = settings.paddleSpeedMultiplier;
float scoreMult = settings.scoreMultiplier;
```

### 获取成就信息
```cpp
auto& achSystem = enhancementManager_.getAchievementSystem();
const auto& allAchs = achSystem.getAllAchievements();
for (const auto& ach : allAchs) {
    if (ach.unlocked) {
        // 显示已解锁成就
    }
}
```

### 获取统计数据
```cpp
const auto& stats = enhancementManager_.getStatisticsTracker().getStatistics();
int thisRunBricks = stats.bricksDestroyedThisRun;
int lifetimeBricks = stats.lifetimeBricksDestroyed;
int bestScore = stats.lifetimeBestScore;
float accuracy = stats.getAccuracy();
```

### 获取排行榜
```cpp
auto topScores = enhancementManager_.getLeaderboardSystem().getTopScores(10);
for (const auto& entry : topScores) {
    // 显示排名
}
```

---

## 🔧 配置参考

### 启用/禁用系统
编辑 `enhancements_config.json`:
```json
{
  "enhancements": {
    "difficulty": { "enabled": true },
    "combo": { "enabled": true },
    "achievements": { "enabled": true },
    "sound": { "enabled": true }
  }
}
```

### 调整音量
```json
"sound": {
  "enabled": true,
  "master_volume": 1.0,
  "sfx_volume": 1.0,
  "music_volume": 0.8
}
```

### 难度参数
```cpp
// 在DifficultyConfig.cpp中修改
settings_.ballSpeedMultiplier = 1.35f;  // 困难模式球速
settings_.paddleWidthMultiplier = 0.85f;  // 困难模式挡板宽度
```

---

## ⚠️ 常见错误

### ❌ 错误：未初始化管理器
```cpp
// 错误
enhancementManager_.onBrickDestroyed();  // Crash!

// 正确
enhancementManager_.initialize();
enhancementManager_.onBrickDestroyed();
```

### ❌ 错误：重复计算倍数
```cpp
// 错误
void Update() {
    ballSpeed = baseBallSpeed * difficulty.ballSpeedMultiplier;  // 每帧重算!
}

// 正确
void ChangeDifficulty() {
    ballSpeed = baseBallSpeed * difficulty.ballSpeedMultiplier;  // 改变时计算
}
```

### ❌ 错误：事件顺序混乱
```cpp
// 错误
SaveProfile();
onBrickDestroyed();

// 正确
onBrickDestroyed();
SaveProfile();
```

---

## 📱 移动端集成

```cpp
#include "MobileInputHandler.hpp"

MobileInputHandler mobileInput;
mobileInput.initialize();

void Update() {
    mobileInput.update();
    Vector2 paddleControl = mobileInput.getPaddleControlInput();
    // 应用到挡板位置
}
```

---

## 🌐 在线功能

```cpp
#include "OnlineLeaderboardClient.hpp"

OnlineLeaderboardClient online;
online.initialize("https://your-server.com");

// 提交分数
LeaderboardEntry entry;
entry.score = 50000;
online.submitScore(entry);

// 获取排行榜
online.fetchGlobalLeaderboard();
```

---

## 💾 回放功能

```cpp
#include "ReplaySystem.hpp"

ReplaySystem replay;

// 记录
replay.startRecording();
// ... 游戏循环 ...
replay.stopRecording();
replay.saveReplay("my_game.replay");

// 回放
replay.loadReplay("my_game.replay");
replay.startPlayback();
replay.update(deltaSeconds);
```

---

## 📚 完整文档

- **ENHANCEMENTS.md** - 详细功能说明
- **INTEGRATION_GUIDE.md** - 完整集成指南
- **IMPLEMENTATION_GUIDE.md** - 最佳实践
- **PROJECT_SUMMARY.md** - 项目总结

---

**最后更新**: 2024  
**版本**: 1.0  
**状态**: ✅ 生产就绪
