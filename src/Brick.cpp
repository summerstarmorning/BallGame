#include "Brick.h"

#include <algorithm>

namespace
{
Brick::Shape toShape(int shapeType)
{
    switch (shapeType)
    {
    case 0:
        return Brick::Shape::Rect;
    case 1:
        return Brick::Shape::Rounded;
    case 2:
        return Brick::Shape::Capsule;
    case 3:
        return Brick::Shape::Slant;
    default:
        return Brick::Shape::Rounded;
    }
}
} // namespace

Brick::Brick(float x, float y, float w, float h, int shapeType)
    : rect {x, y, w, h}
    , active(true)
    , shape(toShape(shapeType))
{
}

void Brick::SetShape(int shapeType)
{
    shape = toShape(shapeType);
}

void Brick::Draw(Color c)
{
    if (!active)
    {
        return;
    }

    const Rectangle r = rect;
    const Color stroke = ColorAlpha(BLACK, 0.22F);

    switch (shape)
    {
    case Shape::Rect:
        DrawRectangleRec(r, c);
        DrawRectangleLinesEx(r, 2.0F, stroke);
        break;
    case Shape::Rounded:
        DrawRectangleRounded(r, 0.20F, 6, c);
        DrawRectangleRoundedLines(r, 0.20F, 6, 2.0F, stroke);
        break;
    case Shape::Capsule:
        DrawRectangleRounded(r, 0.48F, 10, c);
        DrawRectangleRoundedLines(r, 0.48F, 10, 2.0F, stroke);
        break;
    case Shape::Slant: {
        const float cut = std::clamp(r.width * 0.14F, 6.0F, 18.0F);
        const Vector2 p1 {r.x + cut, r.y};
        const Vector2 p2 {r.x + r.width, r.y};
        const Vector2 p3 {r.x + r.width - cut, r.y + r.height};
        const Vector2 p4 {r.x, r.y + r.height};
        DrawTriangle(p1, p2, p3, c);
        DrawTriangle(p1, p3, p4, c);
        DrawLineEx(p1, p2, 2.0F, stroke);
        DrawLineEx(p2, p3, 2.0F, stroke);
        DrawLineEx(p3, p4, 2.0F, stroke);
        DrawLineEx(p4, p1, 2.0F, stroke);
        break;
    }
    }
}
