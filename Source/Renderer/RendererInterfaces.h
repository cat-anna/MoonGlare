#pragma once

namespace MoonGlare::Renderer::Interfaces {

class ContextInputHandler {
public:
    virtual void OnKey(int Key, bool Pressed) = 0;
    virtual void OnChar(unsigned int CharOrKey, bool Pressed) = 0;

    virtual void OnScroll(const emath::fvec2 &delta) = 0;
    virtual void OnMouseButton(int Button, bool State) = 0;
    virtual void MouseDelta(const emath::fvec2 &delta) = 0;

    virtual bool CanDropFocus() = 0;
    virtual void OnFocusChange(bool State) = 0;

    virtual bool CanReleaseMouse() = 0;
    virtual void OnMouseHookChange(bool State) = 0;

    virtual bool ShouldClose(bool Focus, bool MouseHook) = 0;
};

class IContext {
public:
    virtual void EnterCharMode()   = 0;
    virtual void ExitCharMode()    = 0;
    virtual void HookMouse()       = 0;
    virtual void ReleaseMouse()    = 0;

    virtual emath::ivec2 GetSize() const = 0;

    virtual void SetInputHandler(ContextInputHandler *iph) = 0;

    virtual void SetPosition(const emath::ivec2 &pos) = 0;
    virtual void SetTitle(const char* Title) = 0;
};

} //namespace MoonGlare::Renderer::Interfaces
