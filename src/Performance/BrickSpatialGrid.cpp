#include "Performance/BrickSpatialGrid.hpp"

#include <algorithm>
#include <cmath>

namespace game
{
void BrickSpatialGrid::rebuild(
    const std::vector<Rectangle>& rectangles,
    const Rectangle& bounds,
    float cellWidth,
    float cellHeight)
{
    clear();

    // Clamp cell dimensions so malformed config values cannot create a zero
    // size grid. Rectangles may span multiple cells, so each covered cell stores
    // the same rectangle index for later broad-phase lookup.
    bounds_ = bounds;
    cellWidth_ = std::max(1.0F, cellWidth);
    cellHeight_ = std::max(1.0F, cellHeight);
    columns_ = std::max(1, (int)std::ceil(bounds_.width / cellWidth_));
    rows_ = std::max(1, (int)std::ceil(bounds_.height / cellHeight_));
    cells_.assign((std::size_t)columns_ * (std::size_t)rows_, {});
    queryMarks_.assign(rectangles.size(), 0U);
    queryStamp_ = 0U;

    for (std::size_t rectangleIndex = 0; rectangleIndex < rectangles.size(); ++rectangleIndex)
    {
        const Rectangle& rect = rectangles[rectangleIndex];
        const int minColumn = clampColumn(rect.x);
        const int maxColumn = clampColumn(rect.x + rect.width);
        const int minRow = clampRow(rect.y);
        const int maxRow = clampRow(rect.y + rect.height);

        for (int row = minRow; row <= maxRow; ++row)
        {
            for (int column = minColumn; column <= maxColumn; ++column)
            {
                cells_[cellIndex(column, row)].push_back(rectangleIndex);
            }
        }
    }
}

void BrickSpatialGrid::clear() noexcept
{
    cells_.clear();
    queryMarks_.clear();
    queryStamp_ = 0U;
    columns_ = 0;
    rows_ = 0;
    bounds_ = Rectangle {};
}

bool BrickSpatialGrid::empty() const noexcept
{
    return cells_.empty() || queryMarks_.empty();
}

void BrickSpatialGrid::queryCircle(const Vector2& center, float radius, std::vector<std::size_t>& outIndices) const
{
    outIndices.clear();
    if (empty())
    {
        return;
    }

    // queryMarks_ avoids duplicate indices when the query circle overlaps
    // several cells that reference the same rectangle. The stamp reset handles
    // the rare uint32 wraparound case without reallocating per query.
    ++queryStamp_;
    if (queryStamp_ == 0U)
    {
        std::fill(queryMarks_.begin(), queryMarks_.end(), 0U);
        queryStamp_ = 1U;
    }

    const int minColumn = clampColumn(center.x - radius);
    const int maxColumn = clampColumn(center.x + radius);
    const int minRow = clampRow(center.y - radius);
    const int maxRow = clampRow(center.y + radius);

    for (int row = minRow; row <= maxRow; ++row)
    {
        for (int column = minColumn; column <= maxColumn; ++column)
        {
            const auto& cell = cells_[cellIndex(column, row)];
            for (std::size_t rectangleIndex : cell)
            {
                if (queryMarks_[rectangleIndex] == queryStamp_)
                {
                    continue;
                }

                queryMarks_[rectangleIndex] = queryStamp_;
                outIndices.push_back(rectangleIndex);
            }
        }
    }

    std::sort(outIndices.begin(), outIndices.end());
}

std::size_t BrickSpatialGrid::cellIndex(int column, int row) const noexcept
{
    return (std::size_t)row * (std::size_t)columns_ + (std::size_t)column;
}

int BrickSpatialGrid::clampColumn(float worldX) const noexcept
{
    if (columns_ <= 1)
    {
        return 0;
    }

    const float relativeX = (worldX - bounds_.x) / cellWidth_;
    return std::clamp((int)std::floor(relativeX), 0, columns_ - 1);
}

int BrickSpatialGrid::clampRow(float worldY) const noexcept
{
    if (rows_ <= 1)
    {
        return 0;
    }

    const float relativeY = (worldY - bounds_.y) / cellHeight_;
    return std::clamp((int)std::floor(relativeY), 0, rows_ - 1);
}
} // namespace game
