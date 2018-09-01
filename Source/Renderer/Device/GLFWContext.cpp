#include "GLFWContext.h"

//#include "Commands/CommandQueue.h"
//#include "Commands/OpenGL/ControllCommands.h"
//#include "Commands/OpenGL/FramebufferCommands.h"

namespace MoonGlare::Renderer::Device {

static bool IsModeEnabled(const GLFWvidmode* mode) {
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

static std::string DumpMode(const GLFWvidmode* mode) {
    char buf[128];
    float aspect = (float)mode->width / (float)mode->height;
    sprintf(buf, "%dx%d@%dHz bits:%dx%dx%d ratio:%f", mode->width, mode->height, mode->refreshRate, mode->redBits, mode->blueBits, mode->greenBits, aspect);
    return buf;
}

static std::string DumpMode(GLFWContext::VideoMode mode) {
    char buf[128];
    float aspect = (float)mode.width / (float)mode.height;
    sprintf(buf, "%dx%d@%dHz ratio:%f", mode.width, mode.height, mode.refreshRate, aspect);
    return buf;
}


//----------------------------------------------------------------------------------

GLFWContext::GLFWContext(const ContextCreationInfo &ctxifo) {
    m_CharMode = false;
    m_MouseHooked = false;
    m_Focused = false;


    if (!CreateWindow(ctxifo)) {
        AddLog(Error, "GLFWContext creation failed!");
        __debugbreak();
        throw "GLFWContext creation failed!";
    }
}

GLFWContext::~GLFWContext() {
    DestroyWindow();
}

//----------------------------------------------------------------------------------

//IContext

emath::ivec2 GLFWContext::GetSize()const {
    return m_Size;
}

void GLFWContext::SetInputHandler(iContextInputHandler *iph) {
    m_InputHandler = iph;
}

void GLFWContext::SetPosition(const emath::ivec2 & pos) {
    assert(this);
    glfwSetWindowPos(m_Window, pos[0], pos[1]);
}

void GLFWContext::SetTitle(const char * Title) {
    assert(Title);
    assert(this);
    glfwSetWindowTitle(m_Window, Title);
}

unsigned GLFWContext::GetRefreshRate() const {
    auto monitor = glfwGetWindowMonitor(m_Window);
    if (!monitor)
        monitor = glfwGetPrimaryMonitor();
    return glfwGetVideoMode(monitor)->refreshRate;
}

void GLFWContext::SetVisible(bool value) {
    if (value)
        glfwShowWindow(m_Window);
    else
        glfwHideWindow(m_Window);
}


//----------------------------------------------------------------------------------

bool GLFWContext::s_GLFWInitialized = false;

bool GLFWContext::InitializeSubSystem() {
    if (s_GLFWInitialized)
        return false;

    glfwSetErrorCallback(&GLFW_ErrorCallback);

    CriticalCheck(glfwInit(), "Unable to initialize GLFW!");
    AddLog(Debug, "GLFW initialized");
    AddLog(System, "GLFW version: " << glfwGetVersionString());
    s_GLFWInitialized = true;

#ifdef DEBUG_DUMP
    DumpMonitors();
#endif
    return true;
}

bool GLFWContext::FinalizeSubSystem() {
    if (!s_GLFWInitialized)
        return false;

    glfwTerminate();
    AddLog(Debug, "GLFW finalized");

    s_GLFWInitialized = false;
    return true;
}

//----------------------------------------------------------------------------------


int GLFWContext::GetMonitorCount() {
    if (!s_GLFWInitialized)
        throw "GetMonitorCount: GLFW is not initialized!";

    int count;
    glfwGetMonitors(&count);
    return count;
}

std::vector<GLFWContext::VideoMode> GLFWContext::GetMonitorModes(int MonitorIndex) {
    if (!s_GLFWInitialized)
        throw "GetMonitorModes: GLFW is not initialized!";

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    if (MonitorIndex < 0 || MonitorIndex >= count)
        return {};

    GLFWmonitor* monitor = monitors[MonitorIndex];
    int modecount;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modecount);

    std::vector<VideoMode> ret;
    ret.reserve(modecount);

    for (int j = 0; j < modecount; ++j) {
        auto glfwmode = modes + j;
        if (IsModeEnabled(glfwmode)) {
            VideoMode mode;
            mode.width = glfwmode->width;
            mode.height = glfwmode->height;
            mode.refreshRate = glfwmode->refreshRate;
            ret.emplace_back(mode);
        }
    }

    return std::move(ret);
}

void GLFWContext::DumpMonitors() {
    if (!s_GLFWInitialized)
        return;

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    AddLogf(System, "Monitors[%d]:", count);
    for (int i = 0; i < count; ++i) {
        int widthMM, heightMM;
        GLFWmonitor* monitor = monitors[i];
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
        auto name = glfwGetMonitorName(monitor);
        bool primary = glfwGetPrimaryMonitor() == monitor;
        auto cmode = glfwGetVideoMode(monitor);

        AddLogf(System, "%-6s %d: Physical size:[%4dx%-4dmm] Current mode:[%30s] Name:[%s]",
            (primary ? "P" : ""), i, widthMM, heightMM, DumpMode(cmode).c_str(), name);
    }
    AddLogf(System, "Video modes:");

    for (int i = 0; i < count; ++i) {
        //GLFWmonitor* monitor = monitors[i];
        AddLogf(System, "Monitor %d:", i);
        auto modes = GetMonitorModes(i);
        int modeindex = 0;
        for (const auto &mode : modes) {
            AddLogf(System, "    %d.%2d: %s", i, modeindex, DumpMode(mode).c_str());
            ++modeindex;
        }
    }
}

//----------------------------------------------------------------------------------

void GLFWContext::MakeCurrent() {
    glfwMakeContextCurrent(m_Window);
}

void GLFWContext::Flush() {
    glfwSwapBuffers(m_Window);
}

#if 0
void GLFWContext::CaptureScreenShot() {
    //TODO: restore back screen shots

    auto *assiface = m_Renderer->GetAssets();
    auto texl = assiface->GetTextureLoader();
    auto tex = texl->AllocateImage(Asset::TextureLoader::PixelFormat::RGB8, { m_Size[0], m_Size[1] });
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glReadPixels(0, 0, m_Size[0], m_Size[1], GL_BGR, GL_UNSIGNED_BYTE, tex.m_Pixels); //(GLenum)tex.m_PixelFormat
    texl->StoreScreenShot(std::move(tex));
}
#endif
//----------------------------------------------------------------------------------

void GLFWContext::DestroyWindow() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);

        m_Window = nullptr;
    }
}

bool GLFWContext::CreateWindow(ContextCreationInfo ctxifo) {
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    glfwWindowHint(GLFW_VISIBLE, ctxifo.visible ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor *monitor = nullptr;

    if (ctxifo.fullScreen) {
        if (ctxifo.monitor < 0) {
            monitor = glfwGetPrimaryMonitor();
        } else {
            int c;
            auto mont = glfwGetMonitors(&c);
            if (c > ctxifo.monitor)
                monitor = mont[ctxifo.monitor];
        }
    }

    if (ctxifo.width == 0 || ctxifo.height == 0) {
        auto mon = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(mon);
        ctxifo.width = mode->width; 
        ctxifo.height = mode->height;
    }

    m_Window = glfwCreateWindow(ctxifo.width, ctxifo.height, ctxifo.title.c_str(), monitor, 0);
    CriticalCheck(m_Window, "Unable to create new window!");
    MakeCurrent();

    m_LastMousePos = CursorPos();
    glfwSetWindowUserPointer(m_Window, this);
    glfwSwapInterval(0);

    glfwGetWindowSize(m_Window, &m_Size[0], &m_Size[1]);

    glfwSetWindowCloseCallback(m_Window, GLFW_CloseCallback);
    glfwSetWindowFocusCallback(m_Window, GLFW_FocusCallback);
//    glfwSetCursorPosCallback(m_Window, GLFW_CursorCallback);
    glfwSetScrollCallback(m_Window, GLFW_ScrollCallback);
    glfwSetMouseButtonCallback(m_Window, GLFW_MouseButtonCallback);

    ExitCharMode();

    return true;
}

//----------------------------------------------------------------------------------

void GLFWContext::Process() {
    glfwPollEvents();

    if (m_MouseHooked && m_InputHandler) {
        auto pos = CursorPos();
        emath::fvec2 delta = pos - m_LastMousePos;
        m_LastMousePos = pos;
        m_InputHandler->MouseDelta(delta);
    }
}

void GLFWContext::EnterCharMode() {
    m_CharMode = true;
    glfwSetCharCallback(m_Window, GLFW_CharModeCharCallback);
    glfwSetKeyCallback(m_Window, GLFW_CharModeKeyCallback);
}

void GLFWContext::ExitCharMode() {
    m_CharMode = false;
    glfwSetCharCallback(m_Window, nullptr);
    glfwSetKeyCallback(m_Window, GLFW_KeyCallback);
}

void GLFWContext::HookMouse() {
    if (m_MouseHooked)
        return;

    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_MouseHooked = true;
    m_LastMousePos = CursorPos();
    if (m_InputHandler) {
        m_InputHandler->OnMouseHookChange(m_MouseHooked);
    }
}

void GLFWContext::ReleaseMouse() {
    if (!m_MouseHooked)
        return;

    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_MouseHooked = false;
    if (m_InputHandler) {
        m_InputHandler->OnMouseHookChange(m_MouseHooked);
        m_InputHandler->MouseDelta(emath::fvec2(0,0));
    }
}

//----------------------------------------------------------------------------------

emath::fvec2 GLFWContext::CursorPos() {
    double x, y;
    glfwGetCursorPos(m_Window, &x, &y);
    return emath::fvec2(x, y);
}

//----------------------------------------------------------------------------------

inline GLFWContext* GetContextFromWindow(GLFWwindow* window) {
    GLFWContext *ptr = (GLFWContext*)glfwGetWindowUserPointer(window);
    RendererAssert(ptr);
    return ptr;
}

void GLFWContext::GLFW_ErrorCallback(int error, const char* description) {
    AddLogf(Error, "GLFW error: code='%d' descr='%s'", error, description);
}

void GLFWContext::GLFW_CloseCallback(GLFWwindow* window) {
    GLFWContext* ctx = GetContextFromWindow(window);

    auto ih = ctx->m_InputHandler;
    if (ih) {
        if (!ih->ShouldClose(ctx->m_Focused, ctx->m_MouseHooked)) {
            glfwSetWindowShouldClose(window, GL_FALSE);
            return;
        }
    }

    //TODO: handle context closed!!!
    //ctx->m_Renderer->Stop();
}

void GLFWContext::GLFW_FocusCallback(GLFWwindow* window, int focus) {
    GLFWContext* ctx = GetContextFromWindow(window);
    bool Active = focus == GL_TRUE;
    ctx->m_Focused = Active;
    DebugLog(Debug, "Window changed focus state to " << std::boolalpha << Active);

    auto ih = ctx->m_InputHandler;
    if (!ih) 
        return;

//    if (!ih->CanDropFocus())
//        return;

    ih->OnFocusChange(Active);

    if (!Active && ctx->m_MouseHooked) {
        if (ih->CanReleaseMouse()) {
            ctx->ReleaseMouse();
        }
    } else {
        ctx->HookMouse();
    }
 //   if (!inactive) {
//        //w->ReleaseMouse();
//        GetApplication()->OnDeactivate();
 //   }
 //   else {
//        GetApplication()->OnActivate();
//        //w->GrabMouse();
//    }
}

void GLFWContext::GLFW_CursorCallback(GLFWwindow *window, double x, double y) {
//    GLFWContext* ctx = GetContextFromWindow(window);
//    Window* w = ((Window*)glfwGetWindowUserPointer(window));
//    vec2 pos = vec2(x, y);
//    vec2 delta = pos - w->m_CursorPos;
//    w->m_CursorPos = pos;
}

void GLFWContext::GLFW_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    GLFWContext* ctx = GetContextFromWindow(window);
    bool state = action == GLFW_PRESS;

    if (!ctx->m_MouseHooked) {
        if (state)
            ctx->HookMouse();
        return;
    }
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnMouseButton(button, state);
}

void GLFWContext::GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    GLFWContext* ctx = GetContextFromWindow(window);

    if (!ctx->m_MouseHooked) 
        return;

    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnScroll(emath::fvec2(xoffset, yoffset));
}

void GLFWContext::GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_UNKNOWN || action == GLFW_REPEAT) {
        return;
    }
    GLFWContext* ctx = GetContextFromWindow(window);
    
    bool Pressed = (action == GLFW_PRESS);

    switch (key) {
#if 0
    case GLFW_KEY_PRINT_SCREEN: {
        if (!Pressed)
            return;
        ctx->CaptureScreenShot();
        return;
    }
#endif
    case GLFW_KEY_ESCAPE:
        if (!Pressed) 
            return;
        if (ctx->m_InputHandler){
            if (ctx->m_MouseHooked) {
                if (ctx->m_InputHandler->CanReleaseMouse()) {
                    ctx->ReleaseMouse();
                }
            } else {
                if (ctx->m_InputHandler->ShouldClose(ctx->m_Focused, ctx->m_MouseHooked)) {
                    glfwSetWindowShouldClose(window, GL_TRUE);
                    GLFW_CloseCallback(window);
                }
            }
            return;
        }
    default:
        if (ctx->m_InputHandler)
            ctx->m_InputHandler->OnKey(key, Pressed);
        return;
    }
}

void GLFWContext::GLFW_CharModeCharCallback(GLFWwindow* window, unsigned int key) {
    GLFWContext* ctx = GetContextFromWindow(window);
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnChar(key, true);
}

void GLFWContext::GLFW_CharModeKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key < GLFW_KEY_ESCAPE) {
        return;
    }
    bool Pressed = (action != GLFW_RELEASE);
    GLFWContext* ctx = GetContextFromWindow(window);
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnChar(key, Pressed);
}

} //namespace MoonGlare::Renderer::Device
