# UI Design System

Updated: 2026-04-20

## Visual Direction

- The front end now uses a glass-cockpit arcade style instead of classroom-demo HUD blocks.
- Core traits: layered translucent panels, cyan/amber accent contrast, animated scanlines, soft volumetric glows, and large display-type headings.
- The layout is split into three presentation bands:
  - HUD cockpit for score, run state, and meta profile.
  - Menu shell for product positioning, progression snapshot, and cosmetic loadout.
  - Result overlays that reuse the same premium visual language.

## Runtime Implementation

- `src/GameRender.cpp` owns the presentation language through a reusable `DrawGlassPanel(...)` helper and a dedicated display-font drawing path.
- English display headings and high-contrast numerics use `assets/fonts/Orbitron-Bold.ttf`.
- Chinese copy still routes through the system CJK font loading path to keep local readability intact.
- Gameplay readability was preserved while upgrading presentation:
  - Active effects moved into a premium stacked modifier panel.
  - Piercing charges are surfaced directly in the HUD and in the live modifier deck.
  - Menu cosmetics still map to the existing click targets, so visual redesign did not break interaction flow.
- The main menu is now Chinese-first instead of English-first:
  - Large Chinese hero title and action copy lead the page.
  - Profile, loadout, and CTA modules are grouped into clearer lobby-style regions.
  - Cosmetic switches are docked into a dedicated configuration card instead of floating like debug toggles.
- The CJK font path now prioritizes `Microsoft YaHei Bold` / `Microsoft YaHei`, and the runtime codepoint set was expanded to cover the current menu, HUD, and result-copy strings so Chinese text no longer falls back to missing glyphs.

## Professional Tooling And Asset Pipeline

- Layout and motion exploration: [Figma](https://www.figma.com/)
- Palette iteration and harmony checks: [Adobe Color](https://color.adobe.com/)
- Integrated display typeface: [Orbitron on Google Fonts](https://fonts.google.com/specimen/Orbitron)
- Commercial-friendly UI blockouts and placeholders: [Kenney UI assets](https://kenney.nl/assets)
- Surface, background, and material references: [ambientCG](https://ambientcg.com/)

## Product References

- [Shatter / Shatter Remastered Deluxe](https://pikpok.com/games/shatter/) for premium arcade presentation and authored-feel packaging.
- [Breakout Recharged](https://atari.com/products/breakout-recharged) for readable modernized brick-breaker UI and power-up framing.

## Next UI Steps

1. Replace placeholder geometric menu previews with authored iconography and panel decals.
2. Add motion easing for menu entrance, panel reveal, and end-of-run stat count-up.
3. Introduce a dedicated settings page and storefront-grade account/progression screens.
4. Build a formal export workflow for UI mockups before further gameplay feature expansion.
