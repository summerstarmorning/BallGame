#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"

class Brick {
public:
    enum class Shape : unsigned char {
        Rect = 0,
        Rounded = 1,
        Capsule = 2,
        Slant = 3,
        Diamond = 4,
        Hex = 5,
    };

private:
    Rectangle rect;
    bool active;
    Shape shape;
    int hitPoints;
    int maxHitPoints;

public:
    Brick(float x, float y, float w, float h, int shapeType = 1, int durability = 1);
    void Draw(Color c = GREEN);
    bool IsActive() const { return active; }
    void SetActive(bool a);
    void SetShape(int shapeType);
    void SetHitPoints(int durability);
    bool ApplyHit(int damage = 1);
    int GetShape() const { return static_cast<int>(shape); }
    int HitPoints() const { return hitPoints; }
    int MaxHitPoints() const { return maxHitPoints; }

    Rectangle GetRect() const { return rect; }
};

#endif
