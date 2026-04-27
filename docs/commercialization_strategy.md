# Commercialization Strategy

Updated: 2026-04-17

## Benchmark Signals

- [Shatter / Shatter Remastered Deluxe](https://pikpok.com/games/shatter/) shows that a brick-breaker can sell as a premium product when it adds authored stages, bosses, audio identity, and strong presentation instead of shipping as a bare arcade clone.
- [Breakout Recharged](https://atari.com/products/breakout-recharged) shows the enduring market value of a recognizable core loop combined with challenge structure, power-up variation, local co-op, and platform breadth.
- [Peglin on the App Store](https://apps.apple.com/us/app/peglin/id6446336622) shows the strongest adjacent model for retention: genre fusion, meta progression, repeatable runs, and a mobile funnel that can support a free entry point with a full-game unlock.

## Product Direction

- Position this project as a replayable action roguelite brick-breaker, not as a classroom clone.
- Target a premium-first core loop for PC and console, then support a mobile free-trial/full-unlock variant if retention metrics justify it.
- Build around four pillars:
  1. Precision feel and readability.
  2. Meta progression that survives each run.
  3. High-variance builds through power-ups, relic-like modifiers, and boss encounters.
  4. Live retention hooks such as daily seeds, challenge ladders, and cosmetic unlocks.

## What This Repository Now Needs

- Persistent player profile and soft-currency economy.
- Stage generation and authored encounter pipeline.
- Boss and elite brick systems.
- Input, save, and settings layers that are platform-safe.
- Analytics/event abstraction before any storefront or monetization SDK is attached.
- Better content packaging, localization, and build separation for PC vs mobile.

## Recommended Revenue Model

- Phase 1: Premium desktop release with demo.
- Phase 2: Mobile free download with a one-time full unlock, if session depth and D1/D7 retention are healthy.
- Phase 3: Cosmetic DLC, soundtrack bundle, and seasonal challenge packs.

## Execution Order

1. Harden the codebase into systems that can safely persist profile and progression data.
2. Add meta progression, unlock economy, and challenge structure.
3. Add content scale: bosses, relics, enemy-like bricks, stage themes, and run modifiers.
4. Add platform and commercial plumbing: storefront assets, telemetry, localization, settings QA, and release automation.
