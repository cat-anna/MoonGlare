#pragma once

#include "../nfRenderer.h"
#include "../iContext.h"
#include "../iRenderDevice.h"

namespace MoonGlare::Renderer::Device {

class GLFWContext final : public iContext {
public:
    GLFWContext(const ContextCreationInfo &ctxifo, iRenderDevice *device);
    ~GLFWContext();

    static bool InitializeSubSystem();
    static bool FinalizeSubSystem();

    static int GetMonitorCount();
    static std::vector<VideoMode> GetMonitorModes(int MonitorIndex);
    static void DumpMonitors();

    //IContext
    void EnterCharMode() override;
    void ExitCharMode() override;
    void HookMouse() override;
    void ReleaseMouse() override;
    emath::ivec2 GetSize() const override;
    void SetInputHandler(iContextInputHandler *iph) override;
    void SetPosition(const emath::ivec2 &pos) override ;
    void SetTitle(const char* Title) override;
    unsigned GetRefreshRate() const override;
    void SetVisible(bool value) override;

    void MakeCurrent();
    void Flush();

    GLFWwindow* GetHandle() {
        return m_Window;
    }

    void Process();

    emath::fvec2 CursorPos();
private:
    emath::ivec2 m_Size;
    GLFWwindow *m_Window = nullptr;
    iContextInputHandler *m_InputHandler = nullptr;
    iRenderDevice *device;

    bool m_CharMode;
    bool m_MouseHooked;
    bool m_Focused;
    emath::fvec2 m_LastMousePos;

    bool CreateWindow(ContextCreationInfo ctxifo);
    void DestroyWindow();

    static bool s_GLFWInitialized;
    static void GLFW_ErrorCallback(int error, const char* description);
    static void GLFW_CloseCallback(GLFWwindow* window);
    static void GLFW_FocusCallback(GLFWwindow* window, int focus);
    static void GLFW_CursorCallback(GLFWwindow *window, double x, double y);
    static void GLFW_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void GLFW_CharModeCharCallback(GLFWwindow* window, unsigned int key);
    static void GLFW_CharModeKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

} //namespace MoonGlare::Renderer::Device
