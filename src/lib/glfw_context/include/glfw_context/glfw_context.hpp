#pragma once

#include "input_handler/input_handler.hpp"
#include "renderer/device_context.hpp"

namespace MoonGlare::GlfwContext {

class GlfwContext final : public Renderer::iDeviceContext {
public:
    GlfwContext();
    ~GlfwContext() override;

    std::unique_ptr<Renderer::iDeviceWindow> CreateWindow(Renderer::WindowCreationInfo window_info,
                                                          InputHandler::iInputHandler *input_handler) override;

    void PoolEvents() override;

    int GetMonitorCount() const override;
    std::vector<Renderer::iDeviceWindow::VideoMode> GetMonitorModes(int MonitorIndex) override;
    void DumpMonitors() override;

private:
    bool glad_initialized = false;
};

} // namespace MoonGlare::GlfwContext