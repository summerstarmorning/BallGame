# Performance Optimization Report

## Bottleneck

The slowest path in the current game loop was brick collision broad-phase under multiball pressure.  
Before optimization, `Game::HandleBallBrickCollision()` scanned every brick for every active ball each frame, which made the cost grow as `O(ball_count * brick_count)`.

I also reviewed the particle system and found avoidable vector compaction, but it was still a secondary issue compared with broad-phase collision when multi-ball and dense levels are active.

## Optimization

I chose Option A: spatial partition for collision detection.

- Added `game::BrickSpatialGrid` in [`include/Performance/BrickSpatialGrid.hpp`](/D:/XXC_Programe/ballGame/include/Performance/BrickSpatialGrid.hpp:1)
- Rebuilt the grid once after level brick generation in [`src/GamePlay.cpp`](/D:/XXC_Programe/ballGame/src/GamePlay.cpp:216)
- Changed ball-vs-brick collision to query only overlapping cells instead of scanning all bricks in [`src/GamePlay.cpp`](/D:/XXC_Programe/ballGame/src/GamePlay.cpp:564)
- Kept candidate indices sorted so collision order stays stable relative to the old full-scan logic

The grid uses fixed cells sized for the current brick distribution, then queries a ball's AABB-covered cells and runs exact `CheckCollisionCircleRec()` only on those candidates.

## Benchmark

Automatic benchmark script:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_collision_benchmark.ps1
```

Raw benchmark data:

- [`docs/performance_benchmark_results.json`](/D:/XXC_Programe/ballGame/docs/performance_benchmark_results.json:1)

Key results from the latest local run:

| Scenario | Before avg frame ms | After avg frame ms | Speedup | Gain |
|---|---:|---:|---:|---:|
| `standard_single_ball` | 0.0002323 | 0.0000307 | 7.58x | 658.0% |
| `dense_multiball` | 0.0065901 | 0.0003852 | 17.11x | 1611.1% |
| `extreme_multiball` | 0.0188336 | 0.0006745 | 27.92x | 2692.1% |

All benchmark scenarios produced matching checksums before and after optimization, so the optimized path preserved the same first-hit collision result in this test harness.

## Conclusion

The main bottleneck was broad-phase brick collision, not rendering or paddle physics.  
Replacing the full brick scan with a spatial grid gave a very large speedup and comfortably exceeded the assignment bonus threshold of 50% improvement.
