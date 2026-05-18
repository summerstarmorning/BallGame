# JSON Levels And Run Save

## Scope

This module upgrades the arcade prototype into a data-driven, multi-level flow:

- Brick layouts load from external JSON files instead of being generated in code.
- The run save stores `score`, `lives`, and `currentLevel`.
- Boot-time continue detection surfaces a resume prompt in the main menu.
- Clearing a stage automatically advances to the next configured level.
- Missing or broken level JSON falls back to a generated default layout and shows a notice.
- Edit mode can modify the current layout at runtime and save it back to JSON.

## Level Files

The active level list lives in [src/GameStyle.hpp](/D:/XXC_Programe/ballGame/src/GameStyle.hpp:1) and currently points to:

- `config/levels/level1.json`
- `config/levels/level2.json`
- `config/levels/level3.json`

Each level file defines:

- `version`
- `name`
- `game.lives`
- `ball.startX/startY/speedX/speedY/radius`
- `paddle.startX/startY/width/height`
- `bricks[]`

Each brick record contains:

- `x`
- `y`
- `width`
- `height`
- `shape`
- `durability`

## Save Format

Runtime progress is stored at `save/run_save.json`.

Current format:

```json
{
  "version": 2,
  "currentLevel": 2,
  "score": 200,
  "lives": 2
}
```

Legacy compatibility:

- If `version` is missing, the loader treats the file as v1.
- Older fields such as `level` are migrated into `currentLevel`.
- The migrated file is immediately rewritten as v2.

## Error Handling

Level loading is guarded in [src/Level/LevelLoader.cpp](/D:/XXC_Programe/ballGame/src/Level/LevelLoader.cpp:1):

- Missing file: fallback layout
- Parse error: fallback layout
- Missing `bricks` array: fallback layout
- Empty `bricks` array: fallback layout

When fallback is used, the game shows a runtime notice instead of crashing.

## Continue Flow

Boot flow:

1. Game reads `save/run_save.json`
2. If a valid save exists, the menu shows `Press C to continue`
3. `C` resumes the saved stage
4. `Enter` or `N` starts a fresh run from level 1

CLI helpers for repeatable demos:

- `breakout_week2.exe --autostart`
- `breakout_week2.exe --autocontinue`
- `breakout_week2.exe --autostart --demo-exit-level 2 --demo-hold-seconds 1.6`
- `breakout_week2.exe --autocontinue --demo-exit-level 2 --demo-hold-seconds 3.0`

## Edit Mode

Edit mode is optional bonus functionality and is enabled during gameplay:

- `E`: enter or leave edit mode
- Left click: add a brick
- Right click: delete a brick
- `1` to `6`: switch brick shape
- `[` and `]`: change durability
- `Ctrl+S`: save current level back to its JSON file

This keeps the authoring path simple while remaining usable for class demos.
