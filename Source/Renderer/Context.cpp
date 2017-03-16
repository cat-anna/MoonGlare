/*
  * Generated by cppsrc.sh
  * On 2017-02-10 21:51:36,38
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "Context.h"
#include "RenderDevice.h"
#include "Renderer.h"

#include "Commands/CommandQueue.h"
#include "Commands/OpenGL/ControllCommands.h"
#include "Commands/OpenGL/FramebufferCommands.h"

namespace MoonGlare::Renderer {

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

static std::string DumpGLFWMode(const GLFWvidmode* mode) {
    char buf[128];
    sprintf(buf, "%dx%d@%dHz bits:%dx%dx%d ratio:%f", mode->width, mode->height, mode->refreshRate, mode->redBits, mode->blueBits, mode->greenBits, (float)mode->width / (float)mode->height);
    return buf;
}


//----------------------------------------------------------------------------------

Context::Context() {
    THROW_ASSERT(s_GLFWInitialized, "GLFW is not initialized. Cannot create window!");

    m_CharMode = false;
    m_MouseHooked = false;
    m_Focused = false;
}

Context::~Context() {
}

//----------------------------------------------------------------------------------

bool Context::s_GLFWInitialized = false;

bool Context::InitializeSubSystem() {
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

bool Context::FinalizeSubSystem() {
    if (!s_GLFWInitialized)
        return false;

    glfwTerminate();
    AddLog(Debug, "GLFW finalized");

    s_GLFWInitialized = false;
    return true;
}

//----------------------------------------------------------------------------------


int Context::GetMonitorCount() {
    if (!s_GLFWInitialized)
        throw "GetMonitorCount: GLFW is not initialized!";

    int count;
    glfwGetMonitors(&count);
    return count;
}

std::vector<GLFWvidmode> Context::GetMonitorModes(int MonitorIndex) {
    if (!s_GLFWInitialized)
        throw "GetMonitorModes: GLFW is not initialized!";

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    if (MonitorIndex < 0 || MonitorIndex >= count)
        return {};

    GLFWmonitor* monitor = monitors[MonitorIndex];
    int modecount;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modecount);

    std::vector<GLFWvidmode> ret;
    ret.reserve(modecount);

    for (int j = 0; j < modecount; ++j) {
        auto mode = modes + j;
        if (IsModeEnabled(mode))
            ret.emplace_back(*mode);
    }

    return std::move(ret);
}

void Context::DumpMonitors() {
    if (!s_GLFWInitialized)
        return;

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    AddLogf(System, "Monitors[%d]:", count);
    for (int i = 0; i < count; ++i) {
        int modecount;
        int widthMM, heightMM;
        GLFWmonitor* monitor = monitors[i];
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
        auto name = glfwGetMonitorName(monitor);
        bool primary = glfwGetPrimaryMonitor() == monitor;
        auto cmode = glfwGetVideoMode(monitor);

        AddLogf(System, "%-6s %d: Physical size:[%4dx%-4dmm] Current mode:[%30s] Name:[%s]",
            (primary ? "P" : ""), i, widthMM, heightMM, DumpGLFWMode(cmode).c_str(), name);
    }
    AddLogf(System, "Video modes:");

    for (int i = 0; i < count; ++i) {
        GLFWmonitor* monitor = monitors[i];
        AddLogf(System, "Monitor %d:", i);
        auto modes = GetMonitorModes(i);
        int modeindex = 0;
        for (const auto &mode: modes) {
            AddLogf(System, "    %d.%2d: %s", i, modeindex, DumpGLFWMode(&mode).c_str());
            ++modeindex;
        }
    }
}

//----------------------------------------------------------------------------------


void Context::MakeCurrent() {
    glfwMakeContextCurrent(m_Window);
}

void Context::Flush() {
    glfwSwapBuffers(m_Window);
}

//----------------------------------------------------------------------------------

void Context::Initialize(const ContextCreationInfo &ctxifo, RendererFacade *renderer, RenderDevice *device) {
    RendererAssert(renderer);
    RendererAssert(device);

    m_Device = device;
    m_Renderer = renderer;
    
    if (!CreateWindow(ctxifo)) {
        AddLog(Error, "Context creation failed!");
        throw "Context creation failed!";
    }

}

void Context::InitializeWindowLayer(Commands::CommandQueue &q, Frame *frame) {
    RendererAssert(frame);
    
    using namespace Commands;

    //clear
    //q.MakeCommand<SetViewport>((GLint)0, (GLint)0, (GLsizei)m_Size[0], (GLsizei)m_Size[1]);
    //q.MakeCommand<FramebufferDrawBind>(InvalidFramebufferHandle);

    //q.SetQueuePreamble();
}

void Context::Finalize() {
    glfwDestroyWindow(m_Window);

    m_Window = nullptr;
}

//----------------------------------------------------------------------------------

bool Context::CreateWindow(ContextCreationInfo ctxifo) {
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    //glfwWindowHint(GLFW_SRGB_CAPABLE, GL_FALSE);

    GLFWmonitor *monitor = nullptr;

    if (ctxifo.FullScreen) {
        if (ctxifo.MonitorIndex < 0) {
            monitor = glfwGetPrimaryMonitor();
        } else {
            int c;
            auto mont = glfwGetMonitors(&c);
            if (c > ctxifo.MonitorIndex)
                monitor = mont[ctxifo.MonitorIndex];
        }
    }

    if (ctxifo.m_Width == 0 || ctxifo.m_Height == 0) {
        auto mon = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(mon);
        ctxifo.m_Width = mode->width; 
        ctxifo.m_Height = mode->height;
    }

    m_Window = glfwCreateWindow(ctxifo.m_Width, ctxifo.m_Height, ctxifo.Title, monitor, 0);
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

void Context::Process() {
    glfwPollEvents();

    if (m_MouseHooked && m_InputHandler) {
        auto pos = CursorPos();
        emath::fvec2 delta = pos - m_LastMousePos;
        m_LastMousePos = pos;
        m_InputHandler->MouseDelta(delta);
    }
}

void Context::EnterCharMode() {
    m_CharMode = true;
    glfwSetCharCallback(m_Window, GLFW_CharModeCharCallback);
    glfwSetKeyCallback(m_Window, GLFW_CharModeKeyCallback);
}

void Context::ExitCharMode() {
    m_CharMode = false;
    glfwSetCharCallback(m_Window, nullptr);
    glfwSetKeyCallback(m_Window, GLFW_KeyCallback);
}

void Context::HookMouse() {
    if (m_MouseHooked)
        return;

    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_MouseHooked = true;
    m_LastMousePos = CursorPos();
    if (m_InputHandler) {
        m_InputHandler->OnMouseHookChange(m_MouseHooked);
    }
}

void Context::ReleaseMouse() {
    if (!m_MouseHooked)
        return;

    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_MouseHooked = false;
    if (m_InputHandler) {
        m_InputHandler->OnMouseHookChange(m_MouseHooked);
        m_InputHandler->MouseDelta(emath::fvec2(0));
    }
}

//----------------------------------------------------------------------------------

emath::fvec2 Context::CursorPos() {
    double x, y;
    glfwGetCursorPos(m_Window, &x, &y);
    return emath::fvec2(x, y);
}

void Context::SetPosition(const emath::ivec2 & pos) {
    RendererAssert(this);
    glfwSetWindowPos(m_Window, pos[0], pos[1]);
}

void Context::SetTitle(const char * Title) {
    RendererAssert(Title);
    RendererAssert(this);
    glfwSetWindowTitle(m_Window, Title);
}

//----------------------------------------------------------------------------------

inline Context* GetContextFromWindow(GLFWwindow* window) {
    Context *ptr = (Context*)glfwGetWindowUserPointer(window);
    RendererAssert(ptr);
    return ptr;
}

void Context::GLFW_ErrorCallback(int error, const char* description) {
    AddLogf(Error, "GLFW error: code='%d' descr='%s'", error, description);
}

void Context::GLFW_CloseCallback(GLFWwindow* window) {
    Context* ctx = GetContextFromWindow(window);

    auto ih = ctx->m_InputHandler;
    if (ih) {
        if (!ih->ShouldClose(ctx->m_Focused, ctx->m_MouseHooked)) {
            glfwSetWindowShouldClose(window, GL_FALSE);
            return;
        }
    }
    ctx->m_Renderer->Stop();
}

void Context::GLFW_FocusCallback(GLFWwindow* window, int focus) {
    Context* ctx = GetContextFromWindow(window);
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

void Context::GLFW_CursorCallback(GLFWwindow *window, double x, double y) {
    Context* ctx = GetContextFromWindow(window);

//    Window* w = ((Window*)glfwGetWindowUserPointer(window));
//    vec2 pos = vec2(x, y);
//    vec2 delta = pos - w->m_CursorPos;
//    w->m_CursorPos = pos;
}

void Context::GLFW_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    Context* ctx = GetContextFromWindow(window);
    bool state = action == GLFW_PRESS;

    if (!ctx->m_MouseHooked) {
        if (state)
            ctx->HookMouse();
        return;
    }
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnMouseButton(button, state);
}

void Context::GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Context* ctx = GetContextFromWindow(window);

    if (!ctx->m_MouseHooked) 
        return;

    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnScroll(emath::fvec2(xoffset, yoffset));
}

void Context::GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_UNKNOWN || action == GLFW_REPEAT) {
        return;
    }
    Context* ctx = GetContextFromWindow(window);
    //if(ctx->)
    bool Pressed = (action == GLFW_PRESS);

    switch (key) {
    case GLFW_KEY_PRINT_SCREEN:
#if 0
        if (!Pressed) 
            return;
        Graphic::GetRenderDevice()->DelayedContextManip([this] {
            auto size = Size();
            auto img = MoonGlare::DataClasses::Texture::AllocateImage(size, MoonGlare::DataClasses::Texture::BPP::RGB);
            Graphic::GetRenderDevice()->ReadScreenPixels(img->image, size, img->value_type);

            char buf[128];

            std::time_t t = std::time(NULL);
            auto tm = *std::localtime(&t);
            sprintf(buf, "ScreenShot_%d-%d-%d_%d-%d-%d.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

            MoonGlare::DataClasses::Texture::AsyncStoreImage(img, buf);
        });
#endif
        return;
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
        [[fallthrough]]
    default:
        if (ctx->m_InputHandler)
            ctx->m_InputHandler->OnKey(key, Pressed);
        return;
    }
}

void Context::GLFW_CharModeCharCallback(GLFWwindow* window, unsigned int key) {
    Context* ctx = GetContextFromWindow(window);
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnChar(key, true);
}

void Context::GLFW_CharModeKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key < GLFW_KEY_ESCAPE) {
        return;
    }
    bool Pressed = (action != GLFW_RELEASE);
    Context* ctx = GetContextFromWindow(window);
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnChar(key, Pressed);
}

} //namespace MoonGlare::Renderer 
