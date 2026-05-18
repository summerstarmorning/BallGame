# Recent Changes

## 2026-04-15

- Reworked paddle-to-ball interaction so paddle movement contributes full 2D relative velocity.
- Added a post-collision direction constraint for the ball.
- The constraint preserves total speed while forcing a healthier vertical component, which prevents long near-horizontal rallies in late gameplay.
- Unified ball-vs-paddle and ball-vs-brick collision response around the same normal-based reflection model.
- Documented the current physics behavior in `docs/architecture.md`.
- Softened paddle-to-ball force transfer, especially on vertical paddle motion, so the ball no longer gets overpowered upward/downward kicks.
- Added a `pierce_ball` power-up that stores piercing charges and lets the ball destroy bricks without bouncing while charge remains.
- Added durable bricks that take multiple hits, with visible durability markers.
- Expanded brick silhouette variety so levels use a broader non-rectangular mix.

## 2026-04-17

- Added a persistent `player_profile.json` flow with best score, highest stage, soft currency, and lifetime progression stats.
- Surfaced commercial meta-progression directly in the HUD through coins and best-score display.
- Added repository-level commercialization guidance based on current benchmark products in `docs/commercialization_strategy.md`.
- Extended the runtime world context so systems can record long-term progression data without hard-coupling to the game loop.
- Rebuilt the front-end presentation layer around a premium glass-panel arcade UI with a dedicated display font, reworked HUD grouping, upgraded menu shell, and stronger run-result overlays.
- Added `docs/ui_design_system.md` to document the visual system, the current professional toolchain, and commercial-facing material sources.

## 2026-04-20

- Added an ENet-backed networking module with explicit host/server and client roles for authoritative LAN play experiments.
- Added packed wire-format serialization with endian normalization and `memcpy` transfer for paddle inputs and state snapshots.
- Added a `network_sync_demo` executable that supports localhost dual-instance testing, packet-loss simulation, and client-side interpolation.
- Documented the networking layer, localhost launch flow, ENet concepts, and future extension path in `docs/networking_module.md`.
- Replaced the temporary raylib-based networking debug UI with an SDL3 + SDL3_ttf presentation layer so the demo can render Chinese text and use a more production-style UI stack.
- Added local cached source archives for SDL3, SDL3_ttf, and FreeType under `third_party/cache` to make the networking demo build stable in this workspace.
- Rebuilt the main menu into a Chinese-first commercial lobby layout with a stronger hero area, a cleaner player snapshot block, and a docked style-switch module.
- Expanded the UI font codepoint set and prioritized YaHei bold/system CJK fonts so the new menu, HUD, and overlay Chinese strings render correctly instead of showing missing glyphs.
- Added `docs/ai_dialogue_example.md` with a reusable simulated AI conversation covering localhost dual-instance testing, serialization, and interpolation Q&A.

## 2026-04-27

- Added a week-8 async loading demo triggered by `L`, built with `std::thread` plus `std::mutex` protected shared state.
- Added a non-blocking loading overlay with live progress so the game can visibly keep rendering while the worker thread runs.
- Applied a visible post-load result by refreshing the theme and brick color palette, which makes the assignment video easy to capture.
- Added `docs/async_loading_module.md` to document the threading design, Raylib main-thread constraint, and the recommended recording flow.
- Added a desktop Windows shortcut for directly launching the game build, with a keyboard hotkey for faster local demos.

## 2026-05-13

- Added a `BrickSpatialGrid` broad-phase helper and switched brick collision from full scans to spatial-grid candidate queries.
- Preserved the old first-hit collision semantics by sorting grid candidates back into brick index order before narrow-phase collision.
- Added a dedicated `collision_benchmark` executable plus `scripts/run_collision_benchmark.ps1` for automated before/after comparison.
- Added `docs/performance_benchmark_results.json`, `docs/performance_optimization_report.md`, and `docs/performance_ai_dialogue.md` for the optimization assignment deliverables.
- Added `scripts/generate_performance_chart.ps1` so the benchmark data can be rendered into a submission-friendly screenshot image.
- Added `scripts/capture_window_screenshot.ps1` so a live game window can be captured into the submission folder without external tools.

## 2026-05-18

- Replaced hardcoded brick generation with JSON-driven level loading and added three shipped layouts under `config/levels/`.
- Added `LevelLoader` fallback handling so missing or malformed level JSON now switches to a generated default layout with an in-game notice.
- Added runtime run-save persistence for `currentLevel`, `score`, and `lives`, plus boot-time continue detection in the main menu.
- Added v1-to-v2 save migration so older save files can be upgraded automatically when loaded.
- Added optional gameplay edit mode with `E`, mouse add/remove, shape switching, durability tuning, and `Ctrl+S` JSON export.
- Added `docs/level_save_system.md`, `docs/level_save_ai_dialogue.md`, and automated PowerShell scripts for recording and submission packaging.
