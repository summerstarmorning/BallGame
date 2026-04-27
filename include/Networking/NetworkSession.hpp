#pragma once

#include <deque>
#include <optional>
#include <random>
#include <string>

#include "Networking/NetworkPackets.hpp"

struct _ENetHost;
struct _ENetPeer;

namespace networking
{
using ENetHost = ::_ENetHost;
using ENetPeer = ::_ENetPeer;

enum class ConnectionState
{
    Disconnected,
    Connecting,
    Connected,
};

struct HostConfig
{
    std::uint16_t listenPort {7777U};
    std::size_t maxPeers {4U};
    float packetLossChance {0.0F};
};

struct ClientConfig
{
    std::string hostIp {"127.0.0.1"};
    std::uint16_t hostPort {7777U};
    float packetLossChance {0.0F};
};

class PacketLossSimulator
{
public:
    PacketLossSimulator();

    void setDropChance(float dropChance);
    bool shouldDrop();

private:
    float dropChance_ {0.0F};
    std::mt19937 randomEngine_;
    std::uniform_real_distribution<float> distribution_ {0.0F, 1.0F};
};

class EnetRuntime
{
public:
    EnetRuntime();
    ~EnetRuntime();

    bool isReady() const;

private:
    bool ready_ {false};
};

class SnapshotInterpolator
{
public:
    void pushSnapshot(const SnapshotEnvelope& snapshot);
    bool sample(double renderTimeSeconds, GameState& outState) const;
    void clear();

private:
    static GameState interpolate(const SnapshotEnvelope& from, const SnapshotEnvelope& to, double renderTimeSeconds);

    std::deque<SnapshotEnvelope> bufferedSnapshots_;
};

class HostSession
{
public:
    HostSession();
    ~HostSession();

    bool start(const HostConfig& config, std::string& errorMessage);
    void stop();
    void service(std::uint32_t timeoutMs = 0U);
    void broadcastState(const GameState& state);

    bool hasRemoteInput() const;
    std::optional<PaddleInput> consumeRemoteInput();
    bool isClientConnected() const;
    ConnectionState connectionState() const;

private:
    void handleReceive(const std::uint8_t* data, std::size_t length);

    ENetHost* host_ {nullptr};
    ENetPeer* clientPeer_ {nullptr};
    ConnectionState connectionState_ {ConnectionState::Disconnected};
    std::optional<PaddleInput> latestRemoteInput_ {};
    PacketLossSimulator packetLossSimulator_ {};
};

class ClientSession
{
public:
    ClientSession();
    ~ClientSession();

    bool connect(const ClientConfig& config, std::string& errorMessage);
    void stop();
    void service(std::uint32_t timeoutMs = 0U);
    void sendInput(const PaddleInput& input);

    bool hasSnapshot() const;
    std::optional<SnapshotEnvelope> consumeSnapshot();
    ConnectionState connectionState() const;

private:
    void handleReceive(const std::uint8_t* data, std::size_t length, double receivedAtSeconds);

    ENetHost* host_ {nullptr};
    ENetPeer* serverPeer_ {nullptr};
    ConnectionState connectionState_ {ConnectionState::Disconnected};
    std::optional<SnapshotEnvelope> latestSnapshot_ {};
    PacketLossSimulator packetLossSimulator_ {};
};

// ENetHost is the local networking endpoint. A server host listens for peers,
// and a client host initiates outgoing connections.
//
// ENetPeer represents one remote connection attached to an ENetHost.
// This is the object we keep when we need to send gameplay packets.
//
// ENetPacket owns the serialized bytes being transferred. After the packet is
// handed to ENet with enet_peer_send(), ENet takes ownership of that packet.
} // namespace networking
