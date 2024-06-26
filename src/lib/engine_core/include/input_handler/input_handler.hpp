#pragma once

#include <math/vector.hpp>

namespace MoonGlare::InputHandler {

class iInputHandler {
public:
    // virtual void OnKey(int Key, bool Pressed) = 0;
    // virtual void OnChar(unsigned int CharOrKey, bool Pressed) = 0;

    // virtual void OnScroll(const math::fvec2 &delta) = 0;
    // virtual void OnMouseButton(int Button, bool State) = 0;
    // virtual void MouseDelta(const math::fvec2 &delta) = 0;

    virtual void OnFocusChange(bool State) = 0;

    // virtual bool CanReleaseMouse() = 0;
    // virtual void OnMouseHookChange(bool State) = 0;

    virtual bool ShouldClose(bool Focus, bool MouseHook) = 0;
};

} // namespace MoonGlare::InputHandler
