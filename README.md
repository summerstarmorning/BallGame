# 阶机破阵 (BallGame)

面向对象程序设计（C++）课程项目，基于 C++17、CMake 和 raylib 开发。项目从经典打砖块玩法出发，加入多关卡、道具、存档、连击反馈、异步加载演示、网络同步 Demo、性能优化与路演展示功能，目标是把一个课堂原型整理成可以编译、演示和继续扩展的完整工程。

## 项目简介

游戏主体是横版街机打砖块。玩家通过挡板反弹小球击碎砖块，推进 12 个 JSON 关卡；每关会自动衔接，局内得分、生命、道具和存档状态会持续记录。相比基础版本，本项目重点强化了三类能力：

- **代码结构化与可读性**：核心玩法、渲染、关卡、道具、存档、粒子、性能优化、网络 Demo 分模块组织。
- **游戏体验**：支持挡板上下移动、冲量式碰撞反馈、连击倍率、贯穿道具、屏幕震动、暂停/继续、暗色/亮色主题。
- **工程实践**：CMake 构建、raylib 资源加载、JSON 数据驱动、空间划分碰撞优化、运行时存档、AI 协作记录与路演文档。

## 功能列表

| 模块 | 说明 |
| --- | --- |
| 基础打砖块 | 挡板移动、小球反弹、砖块破坏、得分、生命、胜负判断 |
| 多关卡系统 | `config/levels/` 下的 JSON 关卡自动加载，支持 12 个阶段 |
| 道具系统 | 扩展挡板、多球、减速、挡板加速、贯穿球等效果 |
| 连击与反馈 | 连击倍率、粒子爆发、边缘火花、命中闪光、短暂停顿、屏幕震动 |
| 本地存档 | 运行中断后可继续关卡进度，玩家档案记录最高分、金币、累计数据 |
| 异步加载演示 | 按 `L` 触发后台线程加载演示，主线程继续渲染进度条 |
| 关卡编辑辅助 | 按 `E` 进入编辑模式，可添加/删除砖块并保存布局 |
| 性能优化 | `BrickSpatialGrid` 用空间网格减少球与砖块的碰撞候选数量 |
| 网络 Demo | `network_sync_demo` 展示权威主机、二进制包、客户端插值同步思路 |

## 操作说明

| 操作 | 按键 |
| --- | --- |
| 开始新游戏 | `Enter` 或 `N` |
| 继续存档 | `C` |
| 挡板左右移动 | `←` / `→` |
| 挡板上下移动 | `↑` / `↓` 或 `W` / `S` |
| 暂停/继续 | `P` 或点击右上角按钮 |
| 异步加载演示 | `L` |
| 关卡编辑模式 | `E` |
| 调试无敌模式 | `Z` |
| 退出 | `Q` 或关闭窗口 |

## 编译运行

### 环境要求

- Windows + Visual Studio 2022 推荐
- CMake >= 3.14
- C++17 编译器
- 项目内已包含 raylib Windows MSVC 依赖和 SDL/SDL_ttf 缓存包

### 构建主游戏

```powershell
cd D:\XXC_Programe\ballGame
cmake -S . -B build_vs -G "Visual Studio 17 2022" -A x64
cmake --build build_vs --config Release --target breakout_week2
.\build_vs\Release\breakout_week2.exe
```

如果已经生成过 `build_vs`，通常只需要：

```powershell
cmake --build build_vs --config Release --target breakout_week2
```

### 运行测试与辅助目标

```powershell
cmake --build build_vs --config Release --target test_collision
.\build_vs\Release\test_collision.exe

cmake --build build_vs --config Release --target collision_benchmark
.\build_vs\Release\collision_benchmark.exe

cmake --build build_vs --config Release --target network_sync_demo
.\build_vs\Release\network_sync_demo.exe
```

### 路演录制辅助参数

```powershell
.\build_vs\Release\breakout_week2.exe --autostart --demo-exit-level 3 --demo-hold-seconds 1.2
.\build_vs\Release\breakout_week2.exe --autostart --capture-dir artifacts\frames
```

## 工程结构

```text
BallGame/
├── src/                    # C++ 源码
│   ├── Game.cpp            # 生命周期、存档、资源、输入、主更新
│   ├── GamePlay.cpp        # 碰撞、关卡推进、道具掉落、连击反馈
│   ├── GameRender.cpp      # UI、粒子、背景、菜单和游戏画面绘制
│   ├── Performance/        # 空间网格等性能优化
│   ├── Networking/         # ENet 网络同步 Demo
│   ├── Save/               # 运行时存档
│   └── tests/              # 碰撞逻辑测试
├── include/                # 头文件与模块接口
├── config/                 # 道具配置与关卡 JSON
├── assets/                 # 图片、字体和美术资源
├── docs/                   # 架构、性能、AI 对话、开发说明
├── scripts/                # 录制、性能图、构建提交辅助脚本
├── CMakeLists.txt          # 构建配置
└── README.md               # 项目说明
```

`build/`、`build_vs/`、用户偏好、玩家档案、压缩包和临时产物不应提交到 Git。

## 关键技术说明

### 1. 冲量式碰撞与自由度

普通打砖块常见做法是碰到挡板后直接取反速度。本项目在 `GamePlay.cpp` 中计算球与矩形的最近点、碰撞法线和穿透深度，再把挡板速度按比例转移给小球。这样挡板上下移动会真实影响反弹方向，避免“死速度”导致的机械手感，也能支撑路演中“自由度提升”的亮点。

### 2. 空间网格性能优化

当砖块数量增加时，逐个检测球与所有砖块会浪费大量计算。`BrickSpatialGrid` 把场景划分为固定网格，砖块登记到覆盖的格子中，查询时只返回小球圆形范围附近的候选砖块。这样保留准确碰撞检测，同时减少无效判断。

### 3. 数据驱动关卡

关卡数据放在 `config/levels/level*.json`。游戏启动或通关后通过 `LevelLoader` 读取关卡配置，如果 JSON 缺失或格式异常，会回退到默认布局并给出提示。这让关卡内容可以独立于 C++ 逻辑修改。

### 4. 异步加载演示

按 `L` 后，游戏启动后台线程模拟资源准备，主线程继续渲染进度条和动画。线程只通过受互斥锁保护的 `AsyncLoadSharedState` 交换进度与结果，避免直接跨线程操作 raylib 纹理资源。

### 5. 网络同步 Demo

`network_sync_demo` 使用 ENet 展示权威主机同步：主机负责球、挡板、比分等状态，客户端发送输入并用快照插值平滑画面。该模块是功能演示，不影响主游戏离线运行。

## AI 协作记录

项目开发中使用 AI 辅助进行架构讨论、调试思路整理、README 和注释规范化。完整整理见 [docs/AI_CONVERSATIONS.md](docs/AI_CONVERSATIONS.md)。三类代表性对话包括：

1. 冲量式碰撞如何替代简单速度取反。
2. 空间网格如何减少砖块碰撞候选。
3. 网络同步为什么采用权威主机、二进制包和客户端插值。

## 团队分工

200字以内版本见 [docs/TEAM_DIVISION.md](docs/TEAM_DIVISION.md)。

付乙淼主要负责程序结构层面：核心逻辑设计、代码重构、性能优化和路演方案设计；夏星晨主要负责游戏体验层面：UI交互、MVP推进、平衡性测试和现场展示。团队共同完成AI协作实践、文档整理、演示材料准备和最终路演。

## 参考文档

- [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)：项目总体总结
- [docs/architecture.md](docs/architecture.md)：架构说明
- [docs/performance_optimization_report.md](docs/performance_optimization_report.md)：性能优化报告
- [docs/networking_module.md](docs/networking_module.md)：网络模块说明
- [docs/level_save_system.md](docs/level_save_system.md)：存档系统说明
- [docs/开发感想.md](docs/开发感想.md)：开发复盘
