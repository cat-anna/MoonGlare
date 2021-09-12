#pragma once

#include <glad/glad.h> // must be before glfw

#include "input_handler/input_handler.hpp"
#include "renderer/device_context.hpp"
#include <GLFW/glfw3.h>
#include <math/vector.hpp>

namespace MoonGlare::GlfwContext {

class GlfwWindowHandler final : public Renderer::iDeviceWindow {
public:
    GlfwWindowHandler(Renderer::WindowCreationInfo window_info,
                      InputHandler::iInputHandler *input_handler);
    ~GlfwWindowHandler();

    // // IContext
    // void EnterCharMode() override;
    // void ExitCharMode() override;
    // void HookMouse() override;
    // void ReleaseMouse() override;
    math::ivec2 GetSize() const override { return size; }
    // void SetInputHandler(iContextInputHandler *iph) override;
    // void SetPosition(const emath::ivec2 &pos) override;
    // void SetTitle(const char *Title) override;
    // unsigned GetRefreshRate() const override;
    // void SetVisible(bool value) override;

    void BindAsRenderTarget() override;
    void SwapBuffers() override;

    // GLFWwindow *GetHandle() { return m_Window; }

    // void Process();

    // emath::fvec2 CursorPos();

private:
    struct GlfwDeleter {
        void operator()(GLFWwindow *w) { glfwDestroyWindow(w); }
    };
    std::unique_ptr<GLFWwindow, GlfwDeleter> glfw_window;
    InputHandler::iInputHandler *const input_handler;

    math::ivec2 size;
    // iContextInputHandler *m_InputHandler = nullptr;
    // iRenderDevice *device;

    // bool m_CharMode;
    bool is_mouse_hooked = false;
    bool is_focused = false;
    // emath::fvec2 m_LastMousePos;

    // bool CreateWindow(ContextCreationInfo ctxifo);
    // void DestroyWindow();

    // static bool s_GLFWInitialized;
    // static void GLFW_ErrorCallback(int error, const char *description);
    static void GLFW_CloseCallback(GLFWwindow *glfw_window);
    static void GLFW_FocusCallback(GLFWwindow *glfw_window, int focus);
    // static void GLFW_CursorCallback(GLFWwindow *window, double x, double y);
    // static void GLFW_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    // static void GLFW_ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    // static void GLFW_CharModeCharCallback(GLFWwindow *window, unsigned int key);
    // static void GLFW_CharModeKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    // static void GLFW_KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};

} // namespace MoonGlare::GlfwContext
