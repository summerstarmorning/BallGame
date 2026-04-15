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
    };

private:
    Rectangle rect;
    bool active;
    Shape shape;

public:
    Brick(float x, float y, float w, float h, int shapeType = 1);
    void Draw(Color c = GREEN);
    bool IsActive() const { return active; }
    void SetActive(bool a) { active = a; }
    void SetShape(int shapeType);
    int GetShape() const { return static_cast<int>(shape); }

    Rectangle GetRect() const { return rect; }
};

#endif
