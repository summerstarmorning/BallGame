#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "raylib.h"

namespace game
{
class BrickSpatialGrid
{
public:
    void rebuild(
        const std::vector<Rectangle>& rectangles,
        const Rectangle& bounds,
        float cellWidth = 112.0F,
        float cellHeight = 52.0F);
    void clear() noexcept;
    bool empty() const noexcept;

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
