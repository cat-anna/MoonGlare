#pragma once

#include "input_handler/input_handler.hpp"
#include <math/vector.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace MoonGlare::Renderer {

struct WindowInfo {
    uint32_t width = 1920;
    uint32_t height = 1080;
    int32_t monitor = -1;
    bool full_screen = false;
};

inline void to_json(nlohmann::json &j, const WindowInfo &p) {
    j = {
        {"width", p.width},
        {"height", p.height},
        {"monitor", p.monitor},
        {"full_screen", p.full_screen},
    };
}

inline void from_json(const nlohmann::json &j, WindowInfo &p) {
    j.at("width").get_to(p.width);
    j.at("height").get_to(p.height);
    j.at("monitor").get_to(p.monitor);
    j.at("full_screen").get_to(p.full_screen);
}

struct WindowCreationInfo : public WindowInfo {
    WindowCreationInfo() = default;
    WindowCreationInfo(WindowInfo wi) : WindowInfo(wi) {}
    std::string title = "MoonGlareEngine";
    bool visible = true;
};

class iDeviceWindow {
public:
    virtual ~iDeviceWindow() = default;

    struct VideoMode {
        int width;
        int height;
        int refreshRate;
    };

    virtual void BindAsRenderTarget() = 0;
    virtual void SwapBuffers() = 0;

    // virtual void EnterCharMode() = 0;
    // virtual void ExitCharMode() = 0;
    // virtual void HookMouse() = 0;
    // virtual void ReleaseMouse() = 0;

    virtual math::ivec2 GetSize() const = 0;
    math::fvec2 GetSizef() {
        auto s = GetSize();
        return {
            static_cast<float>(s[0]),
            static_cast<float>(s[1]),
        };
    }

    // virtual unsigned GetRefreshRate() const = 0;

    // virtual void SetInputHandler(iContextInputHandler *iph) = 0;

    // virtual void SetPosition(const emath::ivec2 &pos) = 0;
    // virtual void SetTitle(const char *Title) = 0;
    // virtual void SetVisible(bool value) = 0;
};

class iDeviceContext {
public:
    virtual ~iDeviceContext() = default;

    virtual std::unique_ptr<iDeviceWindow>
    CreateWindow(WindowCreationInfo window_info, InputHandler::iInputHandler *input_handler) = 0;

    virtual void PoolEvents() = 0;

    virtual int GetMonitorCount() const = 0;
    virtual std::vector<iDeviceWindow::VideoMode> GetMonitorModes(int MonitorIndex) = 0;
    virtual void DumpMonitors() = 0;
};

} // namespace MoonGlare::Renderer
