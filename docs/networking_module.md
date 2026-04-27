# Networking Module

Updated: 2026-04-20

## Scope

- Added an ENet-based authoritative networking module for LAN testing.
- Host acts as the authoritative server and owns simulation, scoring, and snapshot broadcast.
- Client only sends paddle input and renders replicated state.
- Snapshot cadence is fixed to `25 Hz`, which stays inside the requested `20-30` updates per second.
- The networking demo UI now uses `SDL3 + SDL3_ttf` instead of the earlier quick raylib debug shell.
- Chinese UI text is rendered from local Windows CJK fonts such as `msyh.ttc` / `msyhbd.ttc`.

## Code Layout

- `include/Networking/NetworkPackets.hpp`
  - Packet schema, runtime state structs, and serialization API.
- `include/Networking/NetworkSession.hpp`
  - ENet transport wrappers, packet-loss simulator, and interpolation buffer.
- `src/Networking/NetworkPackets.cpp`
  - Packed wire structs, endian-safe float/integer conversion, and `memcpy`-based serialization.
- `src/Networking/NetworkSession.cpp`
  - Host/client session plumbing built on ENet.
- `src/network_demo.cpp`
  - Local dual-instance demo using localhost, SDL3 rendering, Chinese text, and client-side interpolation.

## UI Stack

- `network_sync_demo` now uses SDL3 for window, input, and 2D rendering.
- `SDL3_ttf` is used for scalable text so the demo can render Chinese cleanly.
- To avoid unstable external downloads during local builds, cached source archives live in:
  - `third_party/cache/SDL-release-3.4.4.zip`
  - `third_party/cache/SDL_ttf-release-3.2.2.zip`
  - `third_party/cache/freetype-VER-2-13-2-SDL.zip`

## ENet Notes

- `ENetHost` is the local endpoint.
  - Server host listens for incoming peers.
  - Client host initiates one outbound peer connection.
- `ENetPeer` is the remote connection handle.
  - Host keeps one connected client peer in the current demo.
  - This abstraction can later hold more peers for multiplayer.
- `ENetPacket` owns the serialized byte payload.
  - Snapshots and inputs are sent as raw packed bytes.
  - After `enet_peer_send(...)`, ENet owns packet cleanup.

## Serialization Strategy

- Runtime structs are converted into packed wire structs.
- All integer and float fields are normalized into network byte order before `memcpy`.
- This keeps packet size deterministic and avoids compiler-dependent in-memory layout assumptions.
- Current packet kinds:
  - `PaddleInput`
  - `StateSnapshot`

## Interpolation

- Client buffers received snapshots with receive timestamps.
- Rendering samples the buffer at `current_time - 100 ms`.
- Ball and paddle positions are linearly interpolated between adjacent snapshots.
- This makes packet loss and jitter visibly smoother during localhost tests with simulated drop.

## Localhost Test

1. Start host:
   - `build_vs\Release\network_sync_demo.exe --mode host --port 7777 --simulate-loss 0.20`
2. Start client:
   - `build_vs\Release\network_sync_demo.exe --mode client --server-ip 127.0.0.1 --port 7777 --simulate-loss 0.20`
3. Controls:
   - Host paddle: `A / D`
   - Client paddle: `LEFT / RIGHT`

## Example AI Conversation

- A reusable Chinese dialogue sample for demos, reports, or class presentation notes now lives in `docs/ai_dialogue_example.md`.
- The sample includes direct Q&A about both serialization and interpolation so the networking design can be explained in plain language.

## Extensibility

- `GameState` already stores paddle arrays sized for more than two players.
- Packet/session code is transport-oriented instead of hard-coding gameplay classes, which is the right base for:
  - additional players,
  - authoritative replication of more entities,
  - host migration,
  - reconnect and late-join flows.

## Branch Suggestions

- `feature/networking`
- `feature/serialization`
