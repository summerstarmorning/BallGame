cmake_minimum_required(VERSION 3.10)
project(breakout_week2)

add_executable(breakout_week2
    src/main.cpp
    src/Game.cpp
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


