#pragma once

#include "iContextInputHandler.h"

namespace MoonGlare::Renderer {

struct ContextCreationInfo {
    uint32_t width = 800;
    uint32_t height = 600;
    int32_t monitor = -1;
    bool fullScreen = false;
    const char *title = "";
    bool visible = true;
};

class iContext {
public:
    struct VideoMode {
        int width;
        int height;
        int refreshRate;
    };

    virtual void EnterCharMode() = 0;
    virtual void ExitCharMode() = 0;
    virtual void HookMouse() = 0;
    virtual void ReleaseMouse() = 0;

    virtual emath::ivec2 GetSize() const = 0;
    emath::fvec2 GetSizef() {
        auto s = GetSize();
        return {static_cast<float>(s[0]), static_cast<float>(s[1]), };
    }
    virtual unsigned GetRefreshRate() const = 0;

    virtual void SetInputHandler(iContextInputHandler *iph) = 0;

    virtual void SetPosition(const emath::ivec2 &pos) = 0;
    virtual void SetTitle(const char* Title) = 0;
    virtual void SetVisible(bool value) = 0;

    virtual void Process() = 0;
    virtual void Flush() = 0;
};

} //namespace MoonGlare::Renderer
