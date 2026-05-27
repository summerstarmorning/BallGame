#pragma once

/**
 * 游戏增强系统集成指南
 * 
 * 本文档说明如何在Game类中集成所有新的游戏增强系统。
 */

/*
=== 1. 在Game.h中添加成员变量 ===

在Game类的private部分添加：

    game::GameEnhancementManager enhancementManager_;
    
=== 2. 在Game构造函数中初始化 ===

    Game::Game(...) : ... 
    {
        enhancementManager_.initialize();
    }

=== 3. 在Game析构函数中清理 ===

    Game::~Game()
    {
        enhancementManager_.shutdown();
    }

=== 4. 在Game::Update()中调用系统更新 ===

    void Game::Update()
    {
        float deltaSeconds = GetFrameTime();
        enhancementManager_.update(deltaSeconds);
        
        // 更新难度设置
        const auto& diffSettings = enhancementManager_.getDifficultyManager().getCurrentSettings();
        // 根据diffSettings调整游戏参数
        
        // ... 其他游戏更新逻辑
    }

=== 5. 处理关键游戏事件 ===

在碰撞检测中（HandleBallBrickCollision）：

    void Game::HandleBallBrickCollision(game::Ball& managedBall)
    {
        // ... 碰撞逻辑
        
        if (brickWasHit) {
            enhancementManager_.onBrickDestroyed();
            
            // 应用难度倍数
            const auto& diffSettings = enhancementManager_.getDifficultyManager().getCurrentSettings();
            score += baseScore * diffSettings.scoreMultiplier;
            
            // 检查连击
            if (enhancementManager_.getComboSystem().getCurrentCombo() >= 10) {
                enhancementManager_.onComboMilestone(enhancementManager_.getComboSystem().getCurrentCombo());
            }
        }
    }

在挡板碰撞检测中：

    void Game::HandleBallPaddleCollision(game::Ball& managedBall, const game::Vec2& paddleVelocity)
    {
        // ... 碰撞逻辑
        enhancementManager_.onBallHit();
    }

在关卡完成时：

    void Game::CheckLevelProgress()
    {
        if (allBricksDestroyed()) {
            bool perfect = (lives == levelStartLives);
            enhancementManager_.onLevelComplete(currentLevel, totalLevels, perfect);
        }
    }

在游戏结束时：

    void Game::GameOver()
    {
        enhancementManager_.onGameEnd(score, survivalTime);
        
        // 添加到排行榜
        game::LeaderboardEntry entry;
        entry.playerName = "Player";
        entry.score = score;
        entry.level = currentLevel;
        entry.survivalTime = survivalTime;
        entry.difficulty = enhancementManager_.getDifficultyManager().getCurrentSettings().name;
        entry.achievementCount = enhancementManager_.getAchievementSystem().getUnlockedCount();
        
        enhancementManager_.getLeaderboardSystem().addEntry(entry);
    }

=== 6. 在菜单系统中添加难度选择 ===

在菜单中显示难度选项：

    std::vector<game::DifficultySettings> difficulties = 
        enhancementManager_.getDifficultyManager().getAvailableDifficulties();
    
    for (const auto& diff : difficulties) {
        // 绘制难度按钮
        if (isButtonPressed()) {
            enhancementManager_.getDifficultyManager().setDifficulty(diff.difficulty);
        }
    }

=== 7. 显示统计信息 ===

游戏中显示当前统计：

    const auto& stats = enhancementManager_.getStatisticsTracker().getStatistics();
    // 显示 stats.bricksDestroyedThisRun, stats.powerUpsCollectedThisRun 等

游戏结束时显示详细统计：

    // 单局统计
    DrawText("击碎砖块: " + to_string(stats.bricksDestroyedThisRun), x, y, size, color);
    DrawText("最高连击: " + to_string(stats.maxComboThisRun), x, y+30, size, color);
    DrawText("准度: " + stats.getAccuracyString(), x, y+60, size, color);
    DrawText("生存时间: " + to_string(stats.survivalTimeSeconds) + "s", x, y+90, size, color);

=== 8. 显示成就 ===

创建一个成就面板来显示解锁的成就：

    const auto& achievements = enhancementManager_.getAchievementSystem().getAllAchievements();
    for (const auto& achievement : achievements) {
        if (achievement.unlocked) {
            // 显示已解锁成就，包括emoji图标
            DrawText(achievement.iconEmoji, x, y, fontSize, color);
            DrawText(achievement.nameZh, x+50, y, fontSize, color);
        } else {
            // 显示未解锁成就，显示进度条
            DrawProgressBar(x, y, width, height, achievement.unlockProgress);
        }
    }

=== 9. 显示排行榜 ===

显示高分排行榜：

    auto topScores = enhancementManager_.getLeaderboardSystem().getTopScores(10, false);
    int rank = 1;
    for (const auto& entry : topScores) {
        DrawText(to_string(rank) + ". " + entry.playerName, x, y, fontSize, color);
        DrawText(to_string(entry.score), x+300, y, fontSize, color);
        rank++;
    }

=== 10. 教程系统 ===

在游戏开始前检查教程：

    if (!tutorialCompleted && enhancementManager_.getTutorialSystem().isActive()) {
        // 显示教程提示
        const auto& content = enhancementManager_.getTutorialSystem().getCurrentContent();
        DrawText(content.actionPromptZh, x, y, fontSize, color);
        
        if (userCompletedAction) {
            enhancementManager_.getTutorialSystem().completeCurrentStep();
        }
    }

=== 11. Boss关卡 ===

在特定关卡启用Boss战斗：

    if (isBossLevel) {
        game::BossEnemy boss("Boss Name", 500);
        boss.initialize({screenWidth/2, 200});
        
        // 在游戏循环中更新Boss
        boss.update(deltaSeconds);
        
        // 检测碰撞
        if (ballCollidedWithBoss) {
            boss.takeDamage(1);
            if (boss.isDefeated()) {
                // Boss被击败
            }
        }
    }

*/

// 实现示例（需要在Game.cpp中完成）

/*

void Game::Update()
{
    float deltaSeconds = GetFrameTime();
    
    // 更新所有增强系统
    enhancementManager_.update(deltaSeconds);
    
    // 获取当前难度设置
    const auto& diffSettings = enhancementManager_.getDifficultyManager().getCurrentSettings();
    float ballSpeedMultiplier = diffSettings.ballSpeedMultiplier;
    float paddleSpeedMultiplier = diffSettings.paddleSpeedMultiplier;
    
    // 根据难度调整游戏参数
    // ... 调整球速、挡板速度等
    
    // 正常的游戏更新逻辑
    // ...
}

void Game::HandleBallBrickCollision(game::Ball& managedBall)
{
    // 碰撞逻辑...
    
    if (collision) {
        // 触发事件
        enhancementManager_.onBrickDestroyed();
        
        // 应用分数倍数
        const auto& diffSettings = enhancementManager_.getDifficultyManager().getCurrentSettings();
        score += baseScore * diffSettings.scoreMultiplier;
        
        // 检查连击里程碑
        const auto& combo = enhancementManager_.getComboSystem();
        if (combo.getCurrentCombo() % 10 == 0 && combo.getCurrentCombo() > 0) {
            enhancementManager_.onComboMilestone(combo.getCurrentCombo());
        }
    }
}

*/
