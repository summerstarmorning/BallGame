cmake_minimum_required(VERSION 3.10)
project(breakout_week2)

add_executable(breakout_week2
    main.cpp
    Ball.cpp
    Paddle.cpp
    Brick.cpp
)

# --- Link raylib located inside the project folder ---
# The CMAKE_CURRENT_SOURCE_DIR variable refers to the project's root directory.
# 1. Add the include directory to our target
target_include_directories(breakout_week2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16/include")

# 2. Add the library directory to our target
target_link_directories(breakout_week2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16/lib")

# 3. Link the actual library file to our target
target_link_libraries(breakout_week2 PUBLIC raylib.lib winmm.lib)


