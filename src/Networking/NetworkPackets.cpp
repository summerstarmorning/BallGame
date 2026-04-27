#include "Networking/NetworkPackets.hpp"

#include <algorithm>
#include <cstring>

namespace networking
{
namespace
{
template <typename TValue>
TValue byteswapValue(TValue value);

template <>
std::uint16_t byteswapValue<std::uint16_t>(std::uint16_t value)
{
    return (std::uint16_t)((value >> 8U) | (value << 8U));
}

template <>
std::uint32_t byteswapValue<std::uint32_t>(std::uint32_t value)
{
    return ((value & 0x000000FFU) << 24U) | ((value & 0x0000FF00U) << 8U) | ((value & 0x00FF0000U) >> 8U)
        | ((value & 0xFF000000U) >> 24U);
}

template <>
std::int32_t byteswapValue<std::int32_t>(std::int32_t value)
{
    std::uint32_t raw = 0U;
    std::memcpy(&raw, &value, sizeof(raw));
    raw = byteswapValue(raw);
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

bool isLittleEndian()
{
    constexpr std::uint16_t probe = 0x0001U;
    return *reinterpret_cast<const std::uint8_t*>(&probe) == 0x01U;
}

std::uint16_t toNetworkU16(std::uint16_t value)
{
    return isLittleEndian() ? byteswapValue(value) : value;
}

std::uint32_t toNetworkU32(std::uint32_t value)
{
    return isLittleEndian() ? byteswapValue(value) : value;
}

std::int32_t toNetworkI32(std::int32_t value)
{
    return isLittleEndian() ? byteswapValue(value) : value;
}

std::uint16_t fromNetworkU16(std::uint16_t value)
{
    return toNetworkU16(value);
}

std::uint32_t fromNetworkU32(std::uint32_t value)
{
    return toNetworkU32(value);
}

std::int32_t fromNetworkI32(std::int32_t value)
{
    return toNetworkI32(value);
}

std::uint32_t encodeFloat(float value)
{
    std::uint32_t raw = 0U;
    std::memcpy(&raw, &value, sizeof(raw));
    return toNetworkU32(raw);
}

float decodeFloat(std::uint32_t value)
{
    const std::uint32_t raw = fromNetworkU32(value);
    float decoded = 0.0F;
    std::memcpy(&decoded, &raw, sizeof(decoded));
    return decoded;
}

#pragma pack(push, 1)
struct WireHeader
{
    std::uint32_t magic;
    std::uint16_t version;
    std::uint16_t kind;
};

struct WireVec2
{
    std::uint32_t x;
    std::uint32_t y;
};

struct WirePaddleState
{
    std::uint8_t active;
    std::uint8_t playerId;
    std::uint16_t reserved;
    WireVec2 position;
    WireVec2 velocity;
};

struct WireBallState
{
    std::uint8_t active;
    std::uint8_t reserved[3];
    WireVec2 position;
    WireVec2 velocity;
};

struct WirePaddleInput
{
    WireHeader header;
    std::uint32_t sequence;
    std::uint8_t playerId;
    std::uint8_t reserved[3];
    std::uint32_t paddleCenterX;
    std::uint32_t paddleVelocityX;
};

struct WireGameState
{
    WireHeader header;
    std::uint32_t tick;
    std::uint32_t serverTimeMilliseconds;
    std::uint8_t playerCount;
    std::uint8_t authoritativePlayerId;
    std::uint8_t reserved[2];
    WireBallState ball;
    WirePaddleState paddles[kMaxPlayers];
    std::int32_t scores[kMaxPlayers];
};
#pragma pack(pop)

static_assert(sizeof(WirePaddleInput) == 24U, "WirePaddleInput layout changed unexpectedly.");
static_assert(sizeof(WireGameState) == 136U, "WireGameState layout changed unexpectedly.");
} // namespace

const char* PacketKindName(PacketKind kind)
{
    switch (kind)
    {
    case PacketKind::PaddleInput:
        return "PaddleInput";
    case PacketKind::StateSnapshot:
        return "StateSnapshot";
    }

    return "Unknown";
}

std::vector<std::uint8_t> SerializePaddleInput(const PaddleInput& input)
{
    WirePaddleInput wire {};
    wire.header.magic = toNetworkU32(kProtocolMagic);
    wire.header.version = toNetworkU16(kProtocolVersion);
    wire.header.kind = toNetworkU16(static_cast<std::uint16_t>(PacketKind::PaddleInput));
    wire.sequence = toNetworkU32(input.sequence);
    wire.playerId = input.playerId;
    wire.paddleCenterX = encodeFloat(input.paddleCenterX);
    wire.paddleVelocityX = encodeFloat(input.paddleVelocityX);

    std::vector<std::uint8_t> bytes(sizeof(wire));
    std::memcpy(bytes.data(), &wire, sizeof(wire));
    return bytes;
}

bool DeserializePaddleInput(const std::uint8_t* bytes, std::size_t length, PaddleInput& outInput)
{
    if (bytes == nullptr || length != sizeof(WirePaddleInput))
    {
        return false;
    }

    WirePaddleInput wire {};
    std::memcpy(&wire, bytes, sizeof(wire));
    if (fromNetworkU32(wire.header.magic) != kProtocolMagic
        || fromNetworkU16(wire.header.version) != kProtocolVersion
        || fromNetworkU16(wire.header.kind) != static_cast<std::uint16_t>(PacketKind::PaddleInput))
    {
        return false;
    }

    outInput.sequence = fromNetworkU32(wire.sequence);
    outInput.playerId = wire.playerId;
    outInput.paddleCenterX = decodeFloat(wire.paddleCenterX);
    outInput.paddleVelocityX = decodeFloat(wire.paddleVelocityX);
    return true;
}

std::vector<std::uint8_t> SerializeGameState(const GameState& state)
{
    WireGameState wire {};
    wire.header.magic = toNetworkU32(kProtocolMagic);
    wire.header.version = toNetworkU16(kProtocolVersion);
    wire.header.kind = toNetworkU16(static_cast<std::uint16_t>(PacketKind::StateSnapshot));
    wire.tick = toNetworkU32(state.tick);
    wire.serverTimeMilliseconds = toNetworkU32((std::uint32_t)std::max(0.0F, state.serverTimeSeconds * 1000.0F));
    wire.playerCount = state.playerCount;
    wire.authoritativePlayerId = state.authoritativePlayerId;
    wire.ball.active = state.ball.active ? 1U : 0U;
    wire.ball.position.x = encodeFloat(state.ball.position.x);
    wire.ball.position.y = encodeFloat(state.ball.position.y);
    wire.ball.velocity.x = encodeFloat(state.ball.velocity.x);
    wire.ball.velocity.y = encodeFloat(state.ball.velocity.y);

    for (std::size_t index = 0; index < kMaxPlayers; ++index)
    {
        wire.paddles[index].active = state.paddles[index].active ? 1U : 0U;
        wire.paddles[index].playerId = state.paddles[index].playerId;
        wire.paddles[index].position.x = encodeFloat(state.paddles[index].position.x);
        wire.paddles[index].position.y = encodeFloat(state.paddles[index].position.y);
        wire.paddles[index].velocity.x = encodeFloat(state.paddles[index].velocity.x);
        wire.paddles[index].velocity.y = encodeFloat(state.paddles[index].velocity.y);
        wire.scores[index] = toNetworkI32(state.scores[index]);
    }

    std::vector<std::uint8_t> bytes(sizeof(wire));
    std::memcpy(bytes.data(), &wire, sizeof(wire));
    return bytes;
}

bool DeserializeGameState(const std::uint8_t* bytes, std::size_t length, GameState& outState)
{
    if (bytes == nullptr || length != sizeof(WireGameState))
    {
        return false;
    }

    WireGameState wire {};
    std::memcpy(&wire, bytes, sizeof(wire));
    if (fromNetworkU32(wire.header.magic) != kProtocolMagic
        || fromNetworkU16(wire.header.version) != kProtocolVersion
        || fromNetworkU16(wire.header.kind) != static_cast<std::uint16_t>(PacketKind::StateSnapshot))
    {
        return false;
    }

    outState.tick = fromNetworkU32(wire.tick);
    outState.serverTimeSeconds = (float)fromNetworkU32(wire.serverTimeMilliseconds) / 1000.0F;
    outState.playerCount = wire.playerCount;
    outState.authoritativePlayerId = wire.authoritativePlayerId;
    outState.ball.active = wire.ball.active != 0U;
    outState.ball.position.x = decodeFloat(wire.ball.position.x);
    outState.ball.position.y = decodeFloat(wire.ball.position.y);
    outState.ball.velocity.x = decodeFloat(wire.ball.velocity.x);
    outState.ball.velocity.y = decodeFloat(wire.ball.velocity.y);

    for (std::size_t index = 0; index < kMaxPlayers; ++index)
    {
        outState.paddles[index].active = wire.paddles[index].active != 0U;
        outState.paddles[index].playerId = wire.paddles[index].playerId;
        outState.paddles[index].position.x = decodeFloat(wire.paddles[index].position.x);
        outState.paddles[index].position.y = decodeFloat(wire.paddles[index].position.y);
        outState.paddles[index].velocity.x = decodeFloat(wire.paddles[index].velocity.x);
        outState.paddles[index].velocity.y = decodeFloat(wire.paddles[index].velocity.y);
        outState.scores[index] = fromNetworkI32(wire.scores[index]);
    }

    return true;
}
} // namespace networking
