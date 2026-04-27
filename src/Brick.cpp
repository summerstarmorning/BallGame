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
    case 4:
        return Brick::Shape::Diamond;
    case 5:
        return Brick::Shape::Hex;
    default:
        return Brick::Shape::Rounded;
    }
}

Color mixColor(Color from, Color to, float factor)
{
    const float t = std::clamp(factor, 0.0F, 1.0F);
    return Color {
        (unsigned char)((1.0F - t) * (float)from.r + t * (float)to.r),
        (unsigned char)((1.0F - t) * (float)from.g + t * (float)to.g),
        (unsigned char)((1.0F - t) * (float)from.b + t * (float)to.b),
        (unsigned char)((1.0F - t) * (float)from.a + t * (float)to.a),
    };
}

void drawQuad(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4, Color fill, Color stroke)
{
    DrawTriangle(p1, p2, p3, fill);
    DrawTriangle(p1, p3, p4, fill);
    DrawLineEx(p1, p2, 2.0F, stroke);
    DrawLineEx(p2, p3, 2.0F, stroke);
    DrawLineEx(p3, p4, 2.0F, stroke);
    DrawLineEx(p4, p1, 2.0F, stroke);
}

void drawHex(const Rectangle& rect, Color fill, Color stroke)
{
    const float cut = std::clamp(rect.width * 0.16F, 7.0F, 20.0F);
    const Vector2 p1 {rect.x + cut, rect.y};
    const Vector2 p2 {rect.x + rect.width - cut, rect.y};
    const Vector2 p3 {rect.x + rect.width, rect.y + rect.height * 0.5F};
    const Vector2 p4 {rect.x + rect.width - cut, rect.y + rect.height};
    const Vector2 p5 {rect.x + cut, rect.y + rect.height};
    const Vector2 p6 {rect.x, rect.y + rect.height * 0.5F};

    DrawTriangle(p1, p2, p3, fill);
    DrawTriangle(p1, p3, p4, fill);
    DrawTriangle(p1, p4, p5, fill);
    DrawTriangle(p1, p5, p6, fill);

    DrawLineEx(p1, p2, 2.0F, stroke);
    DrawLineEx(p2, p3, 2.0F, stroke);
    DrawLineEx(p3, p4, 2.0F, stroke);
    DrawLineEx(p4, p5, 2.0F, stroke);
    DrawLineEx(p5, p6, 2.0F, stroke);
    DrawLineEx(p6, p1, 2.0F, stroke);
}
} // namespace

Brick::Brick(float x, float y, float w, float h, int shapeType, int durability)
    : rect {x, y, w, h}
    , active(true)
    , shape(toShape(shapeType))
    , hitPoints(1)
    , maxHitPoints(1)
{
    SetHitPoints(durability);
}

void Brick::SetActive(bool a)
{
    active = a;
    if (!active)
    {
        hitPoints = 0;
    }
    else if (hitPoints <= 0)
    {
        hitPoints = std::max(1, maxHitPoints);
    }
}

void Brick::SetShape(int shapeType)
{
    shape = toShape(shapeType);
}

void Brick::SetHitPoints(int durability)
{
    maxHitPoints = std::max(1, durability);
    hitPoints = active ? maxHitPoints : 0;
}

bool Brick::ApplyHit(int damage)
{
    if (!active || damage <= 0)
    {
        return false;
    }

    hitPoints = std::max(0, hitPoints - damage);
    if (hitPoints > 0)
    {
        return false;
    }

    active = false;
    return true;
}

void Brick::Draw(Color c)
{
    if (!active)
    {
        return;
    }

    const Rectangle r = rect;
    const float wear = maxHitPoints > 1 ? 1.0F - (float)hitPoints / (float)maxHitPoints : 0.0F;
    const Color fill = mixColor(c, Color {34, 34, 44, 255}, wear * 0.42F);
    const Color highlight = mixColor(fill, WHITE, 0.18F);
    const Color stroke = ColorAlpha(BLACK, 0.26F);

    switch (shape)
    {
    case Shape::Rect:
        DrawRectangleRec(r, fill);
        DrawRectangleGradientV((int)r.x, (int)r.y, (int)r.width, (int)(r.height * 0.45F), ColorAlpha(highlight, 0.26F), BLANK);
        DrawRectangleLinesEx(r, 2.0F, stroke);
        break;
    case Shape::Rounded:
        DrawRectangleRounded(r, 0.20F, 6, fill);
        DrawRectangleRounded(r, 0.20F, 6, ColorAlpha(highlight, 0.08F));
        DrawRectangleRoundedLines(r, 0.20F, 6, 2.0F, stroke);
        break;
    case Shape::Capsule:
        DrawRectangleRounded(r, 0.48F, 10, fill);
        DrawRectangleRounded(r, 0.48F, 10, ColorAlpha(highlight, 0.07F));
        DrawRectangleRoundedLines(r, 0.48F, 10, 2.0F, stroke);
        break;
    case Shape::Slant: {
        const float cut = std::clamp(r.width * 0.14F, 6.0F, 18.0F);
        const Vector2 p1 {r.x + cut, r.y};
        const Vector2 p2 {r.x + r.width, r.y};
        const Vector2 p3 {r.x + r.width - cut, r.y + r.height};
        const Vector2 p4 {r.x, r.y + r.height};
        drawQuad(p1, p2, p3, p4, fill, stroke);
        DrawLineEx(
            Vector2 {r.x + cut * 1.2F, r.y + r.height * 0.24F},
            Vector2 {r.x + r.width - cut * 1.3F, r.y + r.height * 0.24F},
            2.0F,
            ColorAlpha(highlight, 0.36F));
        break;
    }
    case Shape::Diamond: {
        const Vector2 p1 {r.x + r.width * 0.5F, r.y};
        const Vector2 p2 {r.x + r.width, r.y + r.height * 0.5F};
        const Vector2 p3 {r.x + r.width * 0.5F, r.y + r.height};
        const Vector2 p4 {r.x, r.y + r.height * 0.5F};
        drawQuad(p1, p2, p3, p4, fill, stroke);
        DrawLineEx(
            Vector2 {r.x + r.width * 0.28F, r.y + r.height * 0.5F},
            Vector2 {r.x + r.width * 0.72F, r.y + r.height * 0.5F},
            2.0F,
            ColorAlpha(highlight, 0.40F));
        break;
    }
    case Shape::Hex:
        drawHex(r, fill, stroke);
        DrawLineEx(
            Vector2 {r.x + r.width * 0.24F, r.y + r.height * 0.22F},
            Vector2 {r.x + r.width * 0.76F, r.y + r.height * 0.22F},
            2.0F,
            ColorAlpha(highlight, 0.34F));
        break;
    }

    if (maxHitPoints > 1)
    {
        const float pipWidth = 12.0F;
        const float pipHeight = 4.0F;
        const float gap = 4.0F;
        const float totalWidth = (float)maxHitPoints * pipWidth + (float)(maxHitPoints - 1) * gap;
        float startX = r.x + (r.width - totalWidth) * 0.5F;
        const float y = r.y + 6.0F;

        for (int index = 0; index < maxHitPoints; ++index)
        {
            const Rectangle pip {startX + index * (pipWidth + gap), y, pipWidth, pipHeight};
            const Color pipColor = index < hitPoints ? ColorAlpha(WHITE, 0.90F) : ColorAlpha(BLACK, 0.26F);
            DrawRectangleRounded(pip, 0.65F, 4, pipColor);
        }
    }
}
