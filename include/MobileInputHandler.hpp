#pragma once

#include "raylib.h"
#include <vector>

namespace game
{

enum class TouchGestureType
{
    SINGLE_TAP,
    DOUBLE_TAP,
    LONG_PRESS,
    SWIPE_LEFT,
    SWIPE_RIGHT,
    PINCH_ZOOM
};

struct TouchGestureEvent
{
    TouchGestureType type;
    Vector2 position;
    Vector2 delta;
    float pressure;
    float duration;
};

class MobileInputHandler
{
public:
    MobileInputHandler();
    
    void initialize();
    void update();
    
    // 获取触屏控制信息
    Vector2 getPaddleControlInput() const noexcept { return paddleControl_; }
    bool isTouching() const noexcept { return isTouching_; }
    Vector2 getTouchPosition() const noexcept { return touchPosition_; }
    
    // 检测手势
    bool detectGesture(TouchGestureType type, Vector2& position);
    std::vector<TouchGestureEvent> getGestureEvents() const noexcept { return gestureEvents_; }
    
    // 触屏区域检测
    bool isTouchInRectangle(Rectangle rect) const;
    Rectangle getButtonPaddingForMobile(Rectangle desktopRect) const;
    
    // 移动端布局调整
    float getMobileUIScale() const noexcept { return uiScale_; }
    bool isPortraitMode() const noexcept { return isPortrait_; }
    
private:
    Vector2 paddleControl_;
    Vector2 touchPosition_;
    Vector2 lastTouchPosition_;
    bool isTouching_;
    float touchTimer_;
    
    std::vector<TouchGestureEvent> gestureEvents_;
    
    float uiScale_;
    bool isPortrait_;
    float screenDpi_;
    
    void detectPaddleSwipe();
    void detectTapGestures();
    void detectPinchGesture();
    void adjustUIForDevice();
};

} // namespace game
