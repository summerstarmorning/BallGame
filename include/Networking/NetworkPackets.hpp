#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace networking
{
constexpr std::uint32_t kProtocolMagic = 0x42474E54U;
constexpr std::uint16_t kProtocolVersion = 1U;
constexpr std::size_t kMaxPlayers = 4U;
constexpr std::uint8_t kSnapshotChannel = 0U;
constexpr std::uint8_t kInputChannel = 1U;

enum class PacketKind : std::uint16_t
{
    PaddleInput = 1U,
    StateSnapshot = 2U,
};

struct Vec2f
{
    float x {0.0F};
    float y {0.0F};
};

struct PaddleState
{
    bool active {false};
    std::uint8_t playerId {0U};
    Vec2f position {};
    Vec2f velocity {};
};

struct BallState
{
    bool active {true};
    Vec2f position {};
    Vec2f velocity {};
};

struct GameState
{
    std::uint32_t tick {0U};
    float serverTimeSeconds {0.0F};
    std::uint8_t playerCount {2U};
    std::uint8_t authoritativePlayerId {0U};
    BallState ball {};
    std::array<PaddleState, kMaxPlayers> paddles {};
    std::array<std::int32_t, kMaxPlayers> scores {};
};

struct PaddleInput
{
    std::uint32_t sequence {0U};
    std::uint8_t playerId {0U};
    float paddleCenterX {0.0F};
    float paddleVelocityX {0.0F};
};

struct SnapshotEnvelope
{
    GameState state {};
    double receivedAtSeconds {0.0};
};

const char* PacketKindName(PacketKind kind);

std::vector<std::uint8_t> SerializePaddleInput(const PaddleInput& input);
bool DeserializePaddleInput(const std::uint8_t* bytes, std::size_t length, PaddleInput& outInput);

std::vector<std::uint8_t> SerializeGameState(const GameState& state);
bool DeserializeGameState(const std::uint8_t* bytes, std::size_t length, GameState& outState);
} // namespace networking
