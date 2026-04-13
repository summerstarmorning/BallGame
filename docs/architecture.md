# Game Systems Architecture

## Goals

- Extensible power-up pipeline with isolated effect implementations.
- Data-driven tuning through JSON without recompiling gameplay values.
- Multi-instance management for balls, power-ups, and particles using `std::vector`.
- Explicit lifetime handling for temporary effects and transient visuals.
- Debug hooks for collision boxes, logs, and emergency cleanup.

## Module Layout

- `PowerUp`: runtime entity for dropped items, position updates, pickup checks, and state.
- `PowerUpEffect`: behavior contract for any buff/debuff. Concrete classes own effect logic.
- `PowerUpFactory`: maps `PowerUpType` to concrete effect objects and avoids long conditional chains.
- `BallManager`: owns all balls, updates them safely, and decides when the player actually loses a life.
- `ParticleSystem`: owns particle spawning, culling, and capped counts for performance.
- `PowerUpConfigLoader`: converts JSON records into strongly typed gameplay parameters.

## Flow

1. A brick breaks and asks the config repository whether a power-up should drop.
2. `PowerUp` entities fall, emit trail particles, and check paddle overlap.
3. On pickup, `PowerUpFactory` creates the matching `PowerUpEffect`.
4. Active effects are updated each frame and automatically expire when needed.
5. `BallManager` only reports life loss when every ball has left play.

## Extension Path

To add a new power-up:

1. Add a new value to `PowerUpType`.
2. Implement a new `PowerUpEffect` subclass.
3. Register it in `PowerUpFactory`.
4. Add a JSON entry for tuning values.

No existing system should need behavioral rewrites.
