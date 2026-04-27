#include "Networking/NetworkSession.hpp"

#include <algorithm>
#include <cstdlib>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <enet/enet.h>

namespace networking
{
namespace
{
Vec2f lerpVec2(const Vec2f& from, const Vec2f& to, float t)
{
    return Vec2f {
        from.x + (to.x - from.x) * t,
        from.y + (to.y - from.y) * t,
    };
}
} // namespace

PacketLossSimulator::PacketLossSimulator()
    : randomEngine_(std::random_device {}())
{
}

void PacketLossSimulator::setDropChance(float dropChance)
{
    dropChance_ = std::clamp(dropChance, 0.0F, 0.95F);
}

bool PacketLossSimulator::shouldDrop()
{
    return distribution_(randomEngine_) < dropChance_;
}

EnetRuntime::EnetRuntime()
{
    ready_ = enet_initialize() == 0;
}

EnetRuntime::~EnetRuntime()
{
    if (ready_)
    {
        enet_deinitialize();
        ready_ = false;
    }
}

bool EnetRuntime::isReady() const
{
    return ready_;
}

void SnapshotInterpolator::pushSnapshot(const SnapshotEnvelope& snapshot)
{
    bufferedSnapshots_.push_back(snapshot);
    while (bufferedSnapshots_.size() > 8U)
    {
        bufferedSnapshots_.pop_front();
    }
}

bool SnapshotInterpolator::sample(double renderTimeSeconds, GameState& outState) const
{
    if (bufferedSnapshots_.empty())
    {
        return false;
    }

    if (bufferedSnapshots_.size() == 1U || renderTimeSeconds <= bufferedSnapshots_.front().receivedAtSeconds)
    {
        outState = bufferedSnapshots_.front().state;
        return true;
    }

    for (std::size_t index = 1; index < bufferedSnapshots_.size(); ++index)
    {
        const SnapshotEnvelope& previous = bufferedSnapshots_[index - 1U];
        const SnapshotEnvelope& current = bufferedSnapshots_[index];
        if (renderTimeSeconds <= current.receivedAtSeconds)
        {
            outState = interpolate(previous, current, renderTimeSeconds);
            return true;
        }
    }

    outState = bufferedSnapshots_.back().state;
    return true;
}

void SnapshotInterpolator::clear()
{
    bufferedSnapshots_.clear();
}

GameState SnapshotInterpolator::interpolate(
    const SnapshotEnvelope& from,
    const SnapshotEnvelope& to,
    double renderTimeSeconds)
{
    const double span = std::max(0.0001, to.receivedAtSeconds - from.receivedAtSeconds);
    const float t = (float)std::clamp((renderTimeSeconds - from.receivedAtSeconds) / span, 0.0, 1.0);

    GameState result = to.state;
    result.serverTimeSeconds = from.state.serverTimeSeconds + (to.state.serverTimeSeconds - from.state.serverTimeSeconds) * t;
    result.ball.position = lerpVec2(from.state.ball.position, to.state.ball.position, t);
    result.ball.velocity = lerpVec2(from.state.ball.velocity, to.state.ball.velocity, t);

    for (std::size_t index = 0; index < kMaxPlayers; ++index)
    {
        result.paddles[index].position = lerpVec2(from.state.paddles[index].position, to.state.paddles[index].position, t);
        result.paddles[index].velocity = lerpVec2(from.state.paddles[index].velocity, to.state.paddles[index].velocity, t);
    }

    return result;
}

HostSession::HostSession() = default;

HostSession::~HostSession()
{
    stop();
}

bool HostSession::start(const HostConfig& config, std::string& errorMessage)
{
    stop();

    ENetAddress address {};
    address.host = ENET_HOST_ANY;
    address.port = config.listenPort;
    host_ = enet_host_create(&address, config.maxPeers, 2, 0, 0);
    if (host_ == nullptr)
    {
        errorMessage = "Failed to create ENet host for server mode.";
        return false;
    }

    packetLossSimulator_.setDropChance(config.packetLossChance);
    connectionState_ = ConnectionState::Connecting;
    return true;
}

void HostSession::stop()
{
    if (clientPeer_ != nullptr)
    {
        enet_peer_reset(clientPeer_);
        clientPeer_ = nullptr;
    }

    if (host_ != nullptr)
    {
        enet_host_destroy(host_);
        host_ = nullptr;
    }

    latestRemoteInput_.reset();
    connectionState_ = ConnectionState::Disconnected;
}

void HostSession::service(std::uint32_t timeoutMs)
{
    if (host_ == nullptr)
    {
        return;
    }

    ENetEvent event {};
    while (enet_host_service(host_, &event, timeoutMs) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            clientPeer_ = event.peer;
            connectionState_ = ConnectionState::Connected;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            handleReceive(event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            clientPeer_ = nullptr;
            latestRemoteInput_.reset();
            connectionState_ = ConnectionState::Connecting;
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
        }

        timeoutMs = 0U;
    }
}

void HostSession::broadcastState(const GameState& state)
{
    if (host_ == nullptr || clientPeer_ == nullptr || connectionState_ != ConnectionState::Connected)
    {
        return;
    }

    if (packetLossSimulator_.shouldDrop())
    {
        return;
    }

    const std::vector<std::uint8_t> payload = SerializeGameState(state);
    ENetPacket* packet = enet_packet_create(payload.data(), payload.size(), 0U);
    if (packet == nullptr)
    {
        return;
    }

    enet_peer_send(clientPeer_, kSnapshotChannel, packet);
    enet_host_flush(host_);
}

bool HostSession::hasRemoteInput() const
{
    return latestRemoteInput_.has_value();
}

std::optional<PaddleInput> HostSession::consumeRemoteInput()
{
    std::optional<PaddleInput> pending = latestRemoteInput_;
    latestRemoteInput_.reset();
    return pending;
}

bool HostSession::isClientConnected() const
{
    return clientPeer_ != nullptr && connectionState_ == ConnectionState::Connected;
}

ConnectionState HostSession::connectionState() const
{
    return connectionState_;
}

void HostSession::handleReceive(const std::uint8_t* data, std::size_t length)
{
    PaddleInput decoded {};
    if (DeserializePaddleInput(data, length, decoded))
    {
        latestRemoteInput_ = decoded;
    }
}

ClientSession::ClientSession() = default;

ClientSession::~ClientSession()
{
    stop();
}

bool ClientSession::connect(const ClientConfig& config, std::string& errorMessage)
{
    stop();

    host_ = enet_host_create(nullptr, 1, 2, 0, 0);
    if (host_ == nullptr)
    {
        errorMessage = "Failed to create ENet host for client mode.";
        return false;
    }

    ENetAddress address {};
    address.port = config.hostPort;
    if (enet_address_set_host(&address, config.hostIp.c_str()) != 0)
    {
        errorMessage = "Failed to resolve target host name.";
        stop();
        return false;
    }

    serverPeer_ = enet_host_connect(host_, &address, 2, 0U);
    if (serverPeer_ == nullptr)
    {
        errorMessage = "Failed to allocate ENet peer for outbound connection.";
        stop();
        return false;
    }

    packetLossSimulator_.setDropChance(config.packetLossChance);
    connectionState_ = ConnectionState::Connecting;
    return true;
}

void ClientSession::stop()
{
    if (serverPeer_ != nullptr)
    {
        enet_peer_reset(serverPeer_);
        serverPeer_ = nullptr;
    }

    if (host_ != nullptr)
    {
        enet_host_destroy(host_);
        host_ = nullptr;
    }

    latestSnapshot_.reset();
    connectionState_ = ConnectionState::Disconnected;
}

void ClientSession::service(std::uint32_t timeoutMs)
{
    if (host_ == nullptr)
    {
        return;
    }

    ENetEvent event {};
    while (enet_host_service(host_, &event, timeoutMs) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            connectionState_ = ConnectionState::Connected;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            handleReceive(event.packet->data, event.packet->dataLength, enet_time_get() / 1000.0);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            latestSnapshot_.reset();
            serverPeer_ = nullptr;
            connectionState_ = ConnectionState::Disconnected;
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
        }

        timeoutMs = 0U;
    }
}

void ClientSession::sendInput(const PaddleInput& input)
{
    if (host_ == nullptr || serverPeer_ == nullptr || connectionState_ != ConnectionState::Connected)
    {
        return;
    }

    if (packetLossSimulator_.shouldDrop())
    {
        return;
    }

    const std::vector<std::uint8_t> payload = SerializePaddleInput(input);
    ENetPacket* packet = enet_packet_create(payload.data(), payload.size(), 0U);
    if (packet == nullptr)
    {
        return;
    }

    enet_peer_send(serverPeer_, kInputChannel, packet);
    enet_host_flush(host_);
}

bool ClientSession::hasSnapshot() const
{
    return latestSnapshot_.has_value();
}

std::optional<SnapshotEnvelope> ClientSession::consumeSnapshot()
{
    std::optional<SnapshotEnvelope> pending = latestSnapshot_;
    latestSnapshot_.reset();
    return pending;
}

ConnectionState ClientSession::connectionState() const
{
    return connectionState_;
}

void ClientSession::handleReceive(const std::uint8_t* data, std::size_t length, double receivedAtSeconds)
{
    GameState decoded {};
    if (DeserializeGameState(data, length, decoded))
    {
        latestSnapshot_ = SnapshotEnvelope {decoded, receivedAtSeconds};
    }
}
} // namespace networking
