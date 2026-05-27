#include "MobileInputHandler.hpp"
#include <cmath>

namespace game
{

MobileInputHandler::MobileInputHandler()
    : paddleControl_({0.0f, 0.0f}), touchPosition_({0.0f, 0.0f}),
      lastTouchPosition_({0.0f, 0.0f}), isTouching_(false), touchTimer_(0.0f),
      uiScale_(1.0f), isPortrait_(false), screenDpi_(100.0f)
{
}

void MobileInputHandler::initialize()
{
    adjustUIForDevice();
}

void MobileInputHandler::update()
{
    // 更新触屏状态
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        touchPosition_ = GetMousePosition();
        if (!isTouching_)
        {
            isTouching_ = true;
            lastTouchPosition_ = touchPosition_;
            touchTimer_ = 0.0f;
        }
        touchTimer_ += GetFrameTime();
        detectPaddleSwipe();
    }
    else
    {
        if (isTouching_)
        {
            detectTapGestures();
        }
        isTouching_ = false;
        paddleControl_ = {0.0f, 0.0f};
    }
    
    // 清理过期的手势事件
    gestureEvents_.clear();
}

void MobileInputHandler::detectPaddleSwipe()
{
    // 根据水平触摸位置控制挡板
    float screenWidth = static_cast<float>(GetScreenWidth());
    
    if (touchPosition_.x < screenWidth / 3.0f)
    {
        paddleControl_.x = -1.0f;  // 向左
    }
    else if (touchPosition_.x > screenWidth * 2.0f / 3.0f)
    {
        paddleControl_.x = 1.0f;   // 向右
    }
    else
    {
        paddleControl_.x = 0.0f;   // 中间，不动
    }
}

void MobileInputHandler::detectTapGestures()
{
    Vector2 delta = Vector2Subtract(lastTouchPosition_, touchPosition_);
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    
    TouchGestureEvent event;
    event.position = touchPosition_;
    event.delta = delta;
    event.duration = touchTimer_;
    
    // 检测长按
    if (touchTimer_ > 0.5f && distance < 50.0f)
    {
        event.type = TouchGestureType::LONG_PRESS;
        gestureEvents_.push_back(event);
    }
    // 检测滑动
    else if (distance > 100.0f)
    {
        if (delta.x > 0)
        {
            event.type = TouchGestureType::SWIPE_LEFT;
        }
        else
        {
            event.type = TouchGestureType::SWIPE_RIGHT;
        }
        gestureEvents_.push_back(event);
    }
    // 检测单击
    else if (distance < 30.0f)
    {
        event.type = TouchGestureType::SINGLE_TAP;
        gestureEvents_.push_back(event);
    }
}

void MobileInputHandler::detectPinchGesture()
{
    // TODO: 实现多点触控手势检测
}

void MobileInputHandler::adjustUIForDevice()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // 判断设备方向
    isPortrait_ = screenHeight > screenWidth;
    
    // 根据屏幕大小调整UI缩放
    if (screenWidth <= 540)
    {
        uiScale_ = 0.8f;  // 小屏幕
    }
    else if (screenWidth <= 720)
    {
        uiScale_ = 1.0f;  // 中等屏幕
    }
    else
    {
        uiScale_ = 1.2f;  // 大屏幕/平板
    }
    
    // 增大触屏区域的按钮尺寸
    screenDpi_ = 100.0f;
}

bool MobileInputHandler::isTouchInRectangle(Rectangle rect) const
{
    return CheckCollisionPointRec(touchPosition_, rect) && isTouching_;
}

Rectangle MobileInputHandler::getButtonPaddingForMobile(Rectangle desktopRect) const
{
    // 为移动设备增大按钮尺寸
    float padding = 10.0f * uiScale_;
    return Rectangle{
        desktopRect.x - padding,
        desktopRect.y - padding,
        desktopRect.width + padding * 2,
        desktopRect.height + padding * 2
    };
}

bool MobileInputHandler::detectGesture(TouchGestureType type, Vector2& position)
{
    for (const auto& event : gestureEvents_)
    {
        if (event.type == type)
        {
            position = event.position;
            return true;
        }
    }
    return false;
}

} // namespace game
