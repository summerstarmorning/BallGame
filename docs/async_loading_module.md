# Async Loading Demo

Updated: 2026-04-27

## Goal

- Match the week 8 assignment requirement for multithreading and resource management.
- Trigger a visible async loading demo with `L`.
- Keep the main thread rendering smoothly while a worker thread simulates heavy loading.
- Apply a visible state change after loading finishes so the result is easy to capture on video.

## Runtime Behavior

- Press `L` in the main menu or during gameplay.
- A background worker thread starts a simulated resource warmup task.
- The main thread continues drawing the scene and overlays a `Loading` panel with progress.
- After completion:
  - the UI theme toggles,
  - the brick color theme refreshes,
  - a success chip appears near the top of the screen.

## Threading Design

- Worker creation uses `std::thread`.
- Shared loading state uses:
  - `std::mutex`
  - progress float
  - finished flag
  - prepared visual result data
- Main thread polls the shared state every frame in `Update()`.
- All Raylib drawing still stays on the main thread.

## Why This Is Safe

- The worker thread never calls `BeginDrawing()`, `EndDrawing()`, or any `Draw*` API.
- Shared state reads and writes are guarded with `std::lock_guard<std::mutex>`.
- The worker thread is joined before destruction, so the game does not leave a live background thread behind.

## Demo Notes

- Best capture path:
  - launch the game
  - stay in the menu
  - press `L`
  - record the progress bar and the final theme refresh
- The menu also shows a direct hint line for this recording flow.
