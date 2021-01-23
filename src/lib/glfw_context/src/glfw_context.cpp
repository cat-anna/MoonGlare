#include <glad/glad.h>

#include "glfw_context/glfw_context.hpp"
#include "glfw_context/glfw_window_handler.hpp"
#include "renderer/device_context.hpp"
#include <GLFW/glfw3.h>
#include <orbit_logger.h>

namespace MoonGlare::GlfwContext {

namespace {

bool IsModeAllowed(const GLFWvidmode *mode) {
    if (mode->blueBits != 8 || mode->greenBits != 8 || mode->redBits != 8)
        return false;
    if (mode->refreshRate != 60)
        return false;
    if (mode->width < 1024)
        return false;
    if (mode->height < 700)
        return false;
    //	float ratio = (float)mode->width / (float)mode->height;
    //	if (!((ratio >= 1.3f && ratio <= 1.4f) || (ratio >= 1.7f && ratio <= 1.8f)))
    //		return false;
    return true;
}

std::string DumpMode(const GLFWvidmode *mode) {
    char buf[128];
    float aspect = (float)mode->width / (float)mode->height;
    sprintf_s(buf, "%dx%d@%dHz bits:%dx%dx%d ratio:%f", mode->width, mode->height, mode->refreshRate, mode->redBits,
              mode->blueBits, mode->greenBits, aspect);
    return buf;
}

std::string DumpMode(Renderer::iDeviceWindow::VideoMode mode) {
    char buf[128];
    float aspect = (float)mode.width / (float)mode.height;
    sprintf_s(buf, "%dx%d@%dHz ratio:%f", mode.width, mode.height, mode.refreshRate, aspect);
    return buf;
}

void GLFW_ErrorCallback(int error, const char *description) {
    AddLogf(Error, "GLFW error: code='%d' descr='%s'", error, description);
}

} // namespace

GlfwContext::GlfwContext() {
    glfwSetErrorCallback(&GLFW_ErrorCallback);

    if (!glfwInit()) {
        throw std::runtime_error("Unable to initialize GLFW!");
    }

    AddLog(Debug, "GLFW initialized");
    AddLog(System, "GLFW version: " << glfwGetVersionString());

#ifdef DEBUG_DUMP
    DumpMonitors();
#endif
}

GlfwContext::~GlfwContext() {
    glfwTerminate();
    AddLog(Debug, "GLFW finalized");
}

std::unique_ptr<Renderer::iDeviceWindow> GlfwContext::CreateWindow(Renderer::WindowCreationInfo window_info,
                                                                   InputHandler::iInputHandler *input_handler) {
    if (window_info.width == 0 || window_info.height == 0) {
        auto mon = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(mon);
        window_info.width = mode->width;
        window_info.height = mode->height;
    }

    auto window = std::make_unique<GlfwWindowHandler>(std::move(window_info), input_handler);

    if (glad_initialized) {
        return window;
    }
    glad_initialized = true;

    if (!gladLoadGL()) {
        throw std::runtime_error("Unable to initialize GLAD");
    }
    AddLogf(System, "Running on OpenGL %d.%d", GLVersion.major, GLVersion.minor);

    glClearColor(0.0f, 0.0f, 0.f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    glBindTexture(GL_TEXTURE_2D, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const unsigned char texd[] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texd);
    window->SwapBuffers();

    return window;
}

void GlfwContext::PoolEvents() {
    glfwPollEvents();
}

int GlfwContext::GetMonitorCount() const {
    int count;
    glfwGetMonitors(&count);
    return count;
}

std::vector<Renderer::iDeviceWindow::VideoMode> GlfwContext::GetMonitorModes(int MonitorIndex) {
    int count;
    auto **monitors = glfwGetMonitors(&count);

    if (MonitorIndex < 0 || MonitorIndex >= count)
        return {};

    auto *monitor = monitors[MonitorIndex];
    int modecount;
    const auto *modes = glfwGetVideoModes(monitor, &modecount);

    std::vector<Renderer::iDeviceWindow::VideoMode> ret;
    ret.reserve(modecount);

    for (int j = 0; j < modecount; ++j) {
        auto glfw_mode = modes + j;
        if (IsModeAllowed(glfw_mode)) {
            Renderer::iDeviceWindow::VideoMode mode;
            mode.width = glfw_mode->width;
            mode.height = glfw_mode->height;
            mode.refreshRate = glfw_mode->refreshRate;
            ret.emplace_back(mode);
        }
    }

    return std::move(ret);
}

void GlfwContext::DumpMonitors() {
    int count;
    auto **monitors = glfwGetMonitors(&count);

    AddLogf(System, "Monitors[%d]:", count);
    for (int i = 0; i < count; ++i) {
        int widthMM, heightMM;
        auto *monitor = monitors[i];
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
        auto name = glfwGetMonitorName(monitor);
        bool primary = glfwGetPrimaryMonitor() == monitor;
        auto cmode = glfwGetVideoMode(monitor);

        AddLogf(System, "%-6s %d: Physical size:[%4dx%-4dmm] Current mode:[%30s] Name:[%s]", (primary ? "P" : ""), i,
                widthMM, heightMM, DumpMode(cmode).c_str(), name);
    }
    AddLogf(System, "Video modes:");

    for (int i = 0; i < count; ++i) {
        // auto* monitor = monitors[i];
        AddLogf(System, "Monitor %d:", i);
        auto modes = GetMonitorModes(i);
        int modeindex = 0;
        for (const auto &mode : modes) {
            AddLogf(System, "    %d.%2d: %s", i, modeindex, DumpMode(mode).c_str());
            ++modeindex;
        }
    }
}

} // namespace MoonGlare::GlfwContext