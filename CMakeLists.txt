cmake_minimum_required(VERSION 3.14)
project(BallGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    enet_external
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG v1.3.18
    GIT_SHALLOW TRUE
)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_TESTS OFF CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    sdl3_external
    URL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cache/SDL-release-3.4.4.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_Declare(
    sdl3_ttf_external
    URL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cache/SDL_ttf-release-3.2.2.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_MakeAvailable(enet_external)
FetchContent_MakeAvailable(sdl3_external)

if(TARGET SDL3_Headers AND NOT TARGET SDL3::Headers)
    add_library(SDL3::Headers INTERFACE IMPORTED GLOBAL)
    target_link_libraries(SDL3::Headers INTERFACE SDL3_Headers)
endif()

if(TARGET SDL3-static AND NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 INTERFACE IMPORTED GLOBAL)
    target_link_libraries(SDL3::SDL3 INTERFACE SDL3-static)
endif()

if(TARGET SDL3-shared AND NOT TARGET SDL3::SDL3-shared)
    add_library(SDL3::SDL3-shared INTERFACE IMPORTED GLOBAL)
    target_link_libraries(SDL3::SDL3-shared INTERFACE SDL3-shared)
endif()

FetchContent_MakeAvailable(sdl3_ttf_external)

add_executable(breakout_week2
    src/main.cpp
    src/Game.cpp
    src/GamePlay.cpp
    src/GameRender.cpp
    src/Ball.cpp
    src/Paddle.cpp
    src/Brick.cpp
)

# --- Include directories for the project ---
target_include_directories(breakout_week2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- Link raylib located inside the project folder ---
# The CMAKE_CURRENT_SOURCE_DIR variable refers to the project's root directory.
# 1. Add the include directory to our target
target_include_directories(breakout_week2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16/include")

# 2. Add the library directory to our target
target_link_directories(breakout_week2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16/lib")

# 3. Link the actual library file to our target
target_link_libraries(breakout_week2 PUBLIC raylib.lib winmm.lib)
# Copy config.json to build dir
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/config.json" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/config.json" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/config.json" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/config2.json" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/config2.json" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/config2.json" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
# --- Simple test target for collision logic ---
add_executable(test_collision
    src/tests/test_collision.cpp
    src/Ball.cpp
    src/Brick.cpp
)

# include and link same as main target so raylib functions resolve
target_include_directories(test_collision PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(test_collision PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16/include")
target_link_directories(test_collision PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16/lib")
target_link_libraries(test_collision PUBLIC raylib.lib winmm.lib)

add_library(game_systems
    src/BallManager.cpp
    src/Config/PowerUpConfigLoader.cpp
    src/Effects/ExpandPaddleEffect.cpp
    src/Effects/MultiBallEffect.cpp
    src/Effects/PaddleSpeedEffect.cpp
    src/Effects/PierceBallEffect.cpp
    src/Effects/PowerUpEffect.cpp
    src/Effects/SlowBallEffect.cpp
    src/Particles/ParticleSystem.cpp
    src/Profile/PlayerProfile.cpp
    src/PowerUps/PowerUp.cpp
    src/PowerUps/PowerUpFactory.cpp
    src/PowerUps/PowerUpSystem.cpp
)

target_include_directories(game_systems PUBLIC include)
target_link_libraries(breakout_week2 PRIVATE game_systems)

add_library(game_networking
    src/Networking/NetworkPackets.cpp
    src/Networking/NetworkSession.cpp
)

target_include_directories(game_networking PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(game_networking PUBLIC "${enet_external_SOURCE_DIR}/include")
target_link_libraries(game_networking PUBLIC enet)

add_executable(network_sync_demo
    src/network_demo.cpp
)

target_include_directories(network_sync_demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(network_sync_demo PRIVATE game_networking SDL3::SDL3 SDL3_ttf::SDL3_ttf ws2_32)
