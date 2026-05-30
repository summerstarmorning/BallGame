#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "raylib.h"

namespace game
{
/**
 * @brief Spatial acceleration grid for brick collision candidates.
 *
 * Why it exists: checking every ball against every brick is simple but becomes
 * wasteful as the level layout grows. The grid stores the indices of rectangles
 * covered by each fixed-size cell, then queryCircle() returns only nearby brick
 * indices. The caller still performs the final precise circle/rectangle test.
 *
 * Usage: call rebuild() whenever the brick layout changes, then call
 * queryCircle() for each active ball before precise collision handling.
 */
class BrickSpatialGrid
{
public:
    /// Rebuild the grid from current brick rectangles and world bounds.
    void rebuild(
        const std::vector<Rectangle>& rectangles,
        const Rectangle& bounds,
        float cellWidth = 112.0F,
        float cellHeight = 52.0F);
    /// Remove all cells and query stamps; safe to call before rebuilding.
    void clear() noexcept;
    /// True when no usable grid data exists and callers should fall back to a linear scan.
    bool empty() const noexcept;

    /// Fill outIndices with unique rectangle indices whose cells overlap the query circle.
    void queryCircle(const Vector2& center, float radius, std::vector<std::size_t>& outIndices) const;

    std::size_t cellCount() const noexcept { return cells_.size(); }
    float cellWidth() const noexcept { return cellWidth_; }
    float cellHeight() const noexcept { return cellHeight_; }

private:
    std::size_t cellIndex(int column, int row) const noexcept;
    int clampColumn(float worldX) const noexcept;
    int clampRow(float worldY) const noexcept;

    Rectangle bounds_ {};
    float cellWidth_ {112.0F};
    float cellHeight_ {52.0F};
    int columns_ {0};
    int rows_ {0};
    std::vector<std::vector<std::size_t>> cells_ {};
    mutable std::vector<std::uint32_t> queryMarks_ {};
    mutable std::uint32_t queryStamp_ {0};
};
} // namespace game
