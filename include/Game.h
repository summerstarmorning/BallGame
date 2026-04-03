#pragma once

#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>
#include <string>

/**
 * @class Game
 * @brief 游戏核心管理类，负责维护状态机、物理更新、UI渲染与实体对象的交互。
 */
class Game {
private:
    /** @brief 屏幕宽度 */
    int screenWidth;
    /** @brief 屏幕高度 */
    int screenHeight;
    /** @brief 游戏主角：小球 */
    Ball ball;
    /** @brief 玩家控制的挡板 */
    Paddle paddle;
    /** @brief 场景中所有砖块的集合 */
    std::vector<Brick> bricks;

    /** @brief 玩家剩余生命值 */
    int lives;
    /** @brief 当前得分 */
    int score;

    /** @enum GameState
     *  @brief 枚举所有的游戏运行状态 
     */
    enum class GameState { MENU, PLAYING, PAUSED, GAMEOVER };
    /** @brief 记录当前所处的游戏状态 */
    GameState currentState;
    /** @brief 判定是否赢得了本局游戏 */
    bool victory;
    /** @brief 是否收到退出游戏窗口的请求 */
    bool exitWindowRequest;

    /** @brief 是否为深色模式 */
    bool isDarkMode;
    /** @brief 是否处于调试/无敌模式 */
    bool debugMode;

    Color ballColor;
    Color paddleColor;
    Color brickColor;
    int ballColorIndex;
    int paddleColorIndex;
    int brickColorIndex;

    const float HIT_OFFSET_FACTOR = 4.0f;
    const float PADDLE_INFLUENCE = 0.2f;
    const float MAX_H_SPEED = 10.0f;
    float prevPaddleX;

    int currentLevel;

    /**
     * @brief 初始化指定关卡的砖块布局及参数
     * @param levelJsonFile 关卡的 JSON 配置文件路径
     */
    void InitConfigAndBricks(const std::string& levelJsonFile);

    /**
     * @brief 检查小球是否触碰屏幕底部边界区域
     */
    void CheckBottomCollision();

    /**
     * @brief 判定并响应小球与挡板之间的碰撞
     * @param paddleVel 挡板此帧移动速度，用于累加切向力
     */
    void HandlePaddleCollision(float paddleVel);

    /**
     * @brief 遍历处理小球与所有砖块的物理碰撞检查
     */
    void HandleBrickCollision();

    /**
     * @brief 初始化砖块
     */
    void InitBricks();

public:
    /**
     * @brief Game 类的构造函数，初始化游戏窗口相关及启动主菜单
     * @param width  窗口的宽度
     * @param height 窗口的高度
     */
    Game(int width, int height);

    /**
     * @brief 查询当前是否应该关闭主窗口
     * @return 返回 true 代表接受到退出指令
     */
    bool ShouldClose() const;

    /**
     * @brief 处理所有来着用户的输入(如移动按键、UI点击)
     */
    void HandleInput();

    /**
     * @brief 执行游戏核心物理与业务逻辑的统一更新
     */
    void Update();

    /**
     * @brief 对全游戏场景和UI进行当帧画面的渲染
     */
    void Draw();

};
