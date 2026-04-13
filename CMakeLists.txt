cmake_minimum_required(VERSION 3.20)
project(BreakoutSystems LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game_systems
    src/BallManager.cpp
    src/Config/PowerUpConfigLoader.cpp
    src/Effects/ExpandPaddleEffect.cpp
    src/Effects/MultiBallEffect.cpp
    src/Effects/PowerUpEffect.cpp
    src/Effects/SlowBallEffect.cpp
    src/Particles/ParticleSystem.cpp
    src/PowerUps/PowerUp.cpp
    src/PowerUps/PowerUpFactory.cpp
    src/PowerUps/PowerUpSystem.cpp
)

target_include_directories(game_systems PUBLIC include)
