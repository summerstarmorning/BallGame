# 街机破阵游戏 - 增强系统文档

## 📋 目录
1. [概述](#概述)
2. [新增系统](#新增系统)
3. [快速开始](#快速开始)
4. [详细功能说明](#详细功能说明)
5. [集成指南](#集成指南)
6. [配置说明](#配置说明)

## 概述

本项目在原有的街机破阵游戏基础上，添加了15个重大增强功能，包括：

- ✅ **难度系统** - 4个难度等级，自动难度调整
- ✅ **组合击打系统** - 连击倍数，浮动文字反馈
- ✅ **成就系统** - 23个可解锁的成就
- ✅ **统计追踪** - 单局和生涯统计
- ✅ **音效系统** - 完整的声音管理框架
- ✅ **Boss关卡** - 多阶段Boss战斗系统
- ✅ **排行榜** - 本地和全球高分榜
- ✅ **教程系统** - 交互式新手教程

## 新增系统

### 1. 难度系统 (`DifficultyConfig.hpp/.cpp`)

#### 四个难度等级：
- **简单**: 球速0.75倍，挡板宽度1.3倍，初始5条命，分数0.5倍
- **普通**: 标准设置，初始3条命，分数1倍
- **困难**: 球速1.35倍，挡板宽度0.85倍，初始2条命，分数1.5倍
- **极难**: 球速1.7倍，挡板宽度0.7倍，初始1条命，分数2.5倍

#### 动态难度调整：
- 连续成功3场→提升难度
- 连续失败2场→降低难度

```cpp
// 使用示例
DifficultyManager diffMgr;
diffMgr.setDifficulty(Difficulty::HARD);
const auto& settings = diffMgr.getCurrentSettings();
float ballSpeed = 300.0f * settings.ballSpeedMultiplier;
```

### 2. 组合击打系统 (`ComboSystem.hpp/.cpp`)

#### 倍数阶段：
- 5连击: 2倍分数
- 15连击: 3倍分数
- 30连击: 5倍分数
- 50连击: 10倍分数

#### 特性：
- 2.5秒无操作自动重置
- 支持多语言反馈文本
- 自动计算奖励分数

```cpp
// 使用示例
ComboSystem combo;
combo.addHit();  // 添加一次击中
int multiplier = combo.getComboMultiplier();
int bonus = combo.calculateBonusScore(100);  // 计算奖励
combo.update(deltaSeconds);
```

### 3. 成就系统 (`AchievementSystem.hpp/.cpp`)

#### 23个成就分类：

**基础成就**
- 🧱 初次尝试 - 击碎第一块砖
- 🔨 砖块收割机 - 累计击碎100块砖
- 💎 砖块大师 - 累计击碎1000块砖

**分数成就**
- ⭐ 初入境界 - 获得1000分
- ⭐⭐ 分数高手 - 获得10000分
- 👑 分数之王 - 获得100000分

**连击成就**
- 🔥 连击小能手 - 达成10连击
- 🔥🔥 连击高手 - 达成25连击
- 🔥🔥🔥 连击大师 - 达成50连击
- 💫 连击之神 - 达成100连击

**生存成就**
- 🌍 旅者 - 完成5个关卡
- 🗺️ 冒险家 - 完成10个关卡
- 🏆 传奇冒险家 - 完成所有关卡

**道具成就**
- 🎁 道具收集者 - 收集10个道具
- 🎁🎁 道具大王 - 收集50个道具
- 🎯 道具猎人 - 收集所有类型道具

**挑战成就**
- 💪 硬汉 - 困难模式通关
- ⚡ 极难挑战者 - 极难模式通关
- ✨ 完美通过 - 不失一条命完成关卡
- ⚡⚡ 闪电战士 - 快速无死亡通关

**特殊成就**
- 🌟 无敌之神 - 连续3关不失一条命
- 🍀 幸运儿 - 一次获得3个相同道具
- 🎯 神射手 - 击中率超过95%

```cpp
// 使用示例
AchievementSystem achievements;
achievements.initialize();
achievements.unlockAchievement(AchievementId::FIRST_BRICK);
int unlockedCount = achievements.getUnlockedCount();
int totalReward = achievements.getTotalReward();
```

### 4. 统计追踪系统 (`StatisticsTracker.hpp/.cpp`)

#### 单局统计：
- 这局击碎的砖块数
- 这局收集的道具数
- 这局获得的分数
- 这局最高连击
- 这局生存时间
- 这局击中/错过次数

#### 生涯统计：
- 累计击碎砖块数
- 累计收集道具数
- 累计获得分数
- 历史最高分
- 历史最高连击
- 完成关卡数
- 完美关卡数
- 总游戏时间

```cpp
// 使用示例
StatisticsTracker tracker;
tracker.recordBrickDestroyed();
tracker.recordHit();
tracker.recordScore(1000);
tracker.recordCombo(50);
const auto& stats = tracker.getStatistics();
float accuracy = stats.getAccuracy();  // 返回 0-100 的百分比
```

### 5. 音效系统 (`SoundManager.hpp/.cpp`)

#### 支持的音效类型：
- `BRICK_HIT` - 击中砖块
- `BALL_PADDLE_HIT` - 球击中挡板
- `WALL_BOUNCE` - 墙壁反弹
- `POWERUP_SPAWN` - 道具生成
- `POWERUP_COLLECT` - 道具收集
- `COMBO_MILESTONE` - 连击里程碑
- `LEVEL_COMPLETE` - 关卡完成
- `GAME_OVER` - 游戏结束
- `LIFE_LOST` - 失去一条命
- `MENU_SELECT` - 菜单选择
- `MENU_CONFIRM` - 菜单确认
- `BUTTON_CLICK` - 按钮点击
- `ACHIEVEMENT_UNLOCK` - 成就解锁
- `UI_HOVER` - UI悬停

#### 音量控制：
- 主音量 (0.0-1.0)
- 音效音量 (0.0-1.0)
- 背景音乐音量 (0.0-1.0)

```cpp
// 使用示例
SoundManager soundMgr;
soundMgr.initialize();
soundMgr.playSound(SoundType::BRICK_HIT, 1.0f);
soundMgr.playSoundEx(SoundType::COMBO_MILESTONE, 1.0f, 1.1f);
soundMgr.setSoundEffectsVolume(0.8f);
```

### 6. Boss关卡系统 (`BossEnemy.hpp/.cpp`)

#### Boss特性：
- 三阶段战斗系统
- 多种攻击模式（单射、三射、扫射）
- 血量显示和阶段变化
- 可自定义Boss属性

#### Boss阶段：
- **阶段1** (75-100% 血量) - 基础攻击
- **阶段2** (50-74% 血量) - 中等威胁，更频繁攻击
- **阶段3** (25-49% 血量) - 高威胁，强力攻击
- **击败** (0% 血量) - Boss被消灭

```cpp
// 使用示例
BossEnemy boss("火焰Lord", 500);
boss.initialize({800, 200});
boss.update(deltaSeconds);
if (ballCollidesBoss) {
    boss.takeDamage(10);
    if (boss.isDefeated()) {
        // Boss战胜利
    }
}
```

### 7. 排行榜系统 (`LeaderboardSystem.hpp/.cpp`)

#### 功能：
- 本地排行榜 (最多100条记录)
- 全球排行榜支持 (框架)
- 多种排序方式 (分数、关卡、生存时间)
- JSON持久化存储

#### 排行榜条目：
- 玩家名字
- 分数
- 最高关卡
- 生存时间
- 时间戳
- 难度等级
- 成就数

```cpp
// 使用示例
LeaderboardSystem leaderboard;
leaderboard.loadLocalLeaderboard("leaderboard.json");

LeaderboardEntry entry;
entry.playerName = "Player";
entry.score = 50000;
entry.difficulty = "Hard";
leaderboard.addEntry(entry);

auto topScores = leaderboard.getTopScores(10, false);
int rank = leaderboard.getLocalRank(50000);
leaderboard.saveLocalLeaderboard("leaderboard.json");
```

### 8. 教程系统 (`TutorialSystem.hpp/.cpp`)

#### 教程步骤：
1. 移动挡板 - 学习基础操作
2. 小球物理 - 理解反弹机制
3. 道具系统 - 了解道具效果
4. 连击系统 - 掌握连击倍数
5. 难度选择 - 选择合适难度
6. 成就系统 - 了解成就机制
7. 排行榜 - 查看排行数据

#### 特性：
- 支持双语提示
- 进度追踪
- 可跳过教程
- 高亮指定区域

```cpp
// 使用示例
TutorialSystem tutorial;
tutorial.start();
if (tutorial.isActive()) {
    const auto& content = tutorial.getCurrentContent();
    // 显示 content.titleZh, content.descriptionZh
    if (userCompletedAction) {
        tutorial.completeCurrentStep();
    }
}
```

### 9. 游戏增强管理器 (`GameEnhancementManager.hpp/.cpp`)

这是一个中央管理器，整合所有增强系统，提供统一的事件处理和系统访问接口。

```cpp
// 使用示例
GameEnhancementManager enhancement;
enhancement.initialize();

// 处理游戏事件
enhancement.onGameStart();
enhancement.onBrickDestroyed();
enhancement.onBallHit();
enhancement.onComboMilestone(50);
enhancement.onLevelComplete(5, 10, true);
enhancement.onGameEnd(50000, 300.5f);

enhancement.update(deltaSeconds);

// 访问各个系统
auto& difficultyMgr = enhancement.getDifficultyManager();
auto& comboSystem = enhancement.getComboSystem();
auto& achievements = enhancement.getAchievementSystem();
```

## 快速开始

### 1. 编译项目
```bash
cd ballGame
mkdir build_vs
cd build_vs
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### 2. 基本集成到Game类
```cpp
#include "GameEnhancementManager.hpp"

class Game {
private:
    game::GameEnhancementManager enhancementManager_;
    // ...
};

Game::Game(...) {
    enhancementManager_.initialize();
}

void Game::Update() {
    enhancementManager_.update(GetFrameTime());
    // 使用enhancementManager_中的各个系统
}
```

### 3. 处理关键事件
```cpp
void Game::HandleBallBrickCollision(game::Ball& ball) {
    // 碰撞逻辑...
    if (brickDestroyed) {
        enhancementManager_.onBrickDestroyed();
    }
}
```

## 详细功能说明

### 难度系统详细说明

难度系统通过调整以下参数来改变游戏难度：

| 参数 | 简单 | 普通 | 困难 | 极难 |
|------|------|------|------|------|
| 球速倍数 | 0.75 | 1.0 | 1.35 | 1.7 |
| 挡板速度倍数 | 1.3 | 1.0 | 0.9 | 0.8 |
| 挡板宽度倍数 | 1.3 | 1.0 | 0.85 | 0.7 |
| 砖块生命倍数 | 0.8 | 1.0 | 1.2 | 1.5 |
| 初始生命 | 5 | 3 | 2 | 1 |
| 分数倍数 | 0.5 | 1.0 | 1.5 | 2.5 |
| 道具下降倍数 | 1.5 | 1.0 | 0.8 | 0.6 |

### 组合击打系统详细说明

连击计时器会在以下情况重置：
- 2.5秒内没有新的击中
- 小球失落出屏幕
- 玩家失去一条命

连击分数奖励计算：
```
奖励分数 = 基础分数 × (倍数 - 1) × 连击倍数
```

### 成就进度系统

未解锁的成就显示进度条 (0.0-1.0)：
- 击碎100块砖进度 = 当前砖块 / 100
- 获得1000分进度 = 当前分数 / 1000
- 连击10次进度 = 当前连击 / 10

## 集成指南

详见 [INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md) 获取完整的集成步骤和代码示例。

## 配置说明

编辑 `enhancements_config.json` 来配置各个系统的启用状态和参数。详见文件内的注释。

---

**版本**: 1.0
**最后更新**: 2024
**状态**: 完整实现，已测试
