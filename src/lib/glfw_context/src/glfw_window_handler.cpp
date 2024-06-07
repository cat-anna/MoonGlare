#include "glfw_context/glfw_window_handler.hpp"
#include <orbit_logger.h>

namespace MoonGlare::GlfwContext {

namespace {

inline GlfwWindowHandler *GetWindowPointer(GLFWwindow *window) {
    GlfwWindowHandler *ptr = (GlfwWindowHandler *)glfwGetWindowUserPointer(window);
    assert(ptr);
    return ptr;
}

} // namespace

GlfwWindowHandler::GlfwWindowHandler(Renderer::WindowCreationInfo window_info,
                                     InputHandler::iInputHandler *input_handler)
    : input_handler(input_handler) {
    // m_CharMode = false;
    // m_MouseHooked = false;
    // m_Focused = false;

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    // glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    glfwWindowHint(GLFW_VISIBLE, window_info.visible ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor *monitor = nullptr;

    if (window_info.full_screen) {
        if (window_info.monitor < 0) {
            monitor = glfwGetPrimaryMonitor();
        } else {
            int c;
            auto mont = glfwGetMonitors(&c);
            if (c > window_info.monitor)
                monitor = mont[window_info.monitor];
        }
    }

    glfw_window.reset(glfwCreateWindow(window_info.width, window_info.height,
                                       window_info.title.c_str(), monitor, 0));
    if (!glfw_window) {
        throw std::runtime_error("Unable to create new window!");
    }
    glfwSetWindowUserPointer(glfw_window.get(), this);

    BindAsRenderTarget();
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Unable to initialize glad");
    }

    // m_LastMousePos = CursorPos();
    glfwSetWindowUserPointer(glfw_window.get(), this);
    glfwGetWindowSize(glfw_window.get(), &size[0], &size[1]);

    glViewport(0, 0, size[0], size[1]);
    glClearColor(0.0f, 0.0f, 0.f, 0.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // glEnable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapInterval(0);

    glfwSetWindowCloseCallback(glfw_window.get(), GLFW_CloseCallback);
    glfwSetWindowFocusCallback(glfw_window.get(), GLFW_FocusCallback);
    // glfwSetCursorPosCallback(m_Window, GLFW_CursorCallback);
    // glfwSetScrollCallback(m_Window, GLFW_ScrollCallback);
    // glfwSetMouseButtonCallback(m_Window, GLFW_MouseButtonCallback);

    // ExitCharMode();
}

GlfwWindowHandler::~GlfwWindowHandler() {
    glfw_window.reset();
}

void GlfwWindowHandler::BindAsRenderTarget() {
    glfwMakeContextCurrent(glfw_window.get());
}

void GlfwWindowHandler::SwapBuffers() {
    glfwSwapBuffers(glfw_window.get());
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

void GlfwWindowHandler::GLFW_CloseCallback(GLFWwindow *glfw_window) {
    auto *window = GetWindowPointer(glfw_window);

    AddLogf(Debug, "GLFW_CloseCallback");
    auto input_handler = window->input_handler;
    if (!input_handler) {
        glfwSetWindowShouldClose(glfw_window, GL_TRUE);
        AddLogf(Debug, "GLFW_CloseCallback no input handler, ignoring event");
        return;
    }

    if (input_handler->ShouldClose(window->is_focused, window->is_mouse_hooked)) {
        glfwSetWindowShouldClose(glfw_window, GL_TRUE);
        AddLogf(Debug, "GLFW_CloseCallback window is going to be closed");
        return;
    }

    glfwSetWindowShouldClose(glfw_window, GL_FALSE);
    AddLogf(Debug, "GLFW_CloseCallback window is not closing");
}

void GlfwWindowHandler::GLFW_FocusCallback(GLFWwindow *glfw_window, int focus) {
    auto *window = GetWindowPointer(glfw_window);

    AddLogf(Debug, "GLFW_FocusCallback(%d)", focus);
    auto input_handler = window->input_handler;
    if (!input_handler) {
        AddLogf(Debug, "GLFW_FocusCallback no input handler, ignoring event");
        return;
    }

    bool focused = (focus == GL_TRUE);
    window->is_focused = focused;
    input_handler->OnFocusChange(focused);
    AddLog(Debug, "Window changed focus state to " << std::boolalpha << focused);

    //    if (!ih->CanDropFocus())
    //        return;

    // if (!Active && ctx->m_MouseHooked) {
    //     if (ih->CanReleaseMouse()) {
    //         ctx->ReleaseMouse();
    //     }
    // } else {
    //     ctx->HookMouse();
    // }
    //   if (!inactive) {
    //        //w->ReleaseMouse();
    //        GetApplication()->OnDeactivate();
    //   }
    //   else {
    //        GetApplication()->OnActivate();
    //        //w->GrabMouse();
    //    }
}

#if 0

//----------------------------------------------------------------------------------

// IContext

emath::ivec2 GLFWContext::GetSize() const { return m_Size; }

void GLFWContext::SetInputHandler(iContextInputHandler *iph) { m_InputHandler = iph; }

void GLFWContext::SetPosition(const emath::ivec2 &pos) {
    assert(this);
    glfwSetWindowPos(m_Window, pos[0], pos[1]);
}

void GLFWContext::SetTitle(const char *Title) {
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

void GLFWContext::Process() {
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
        m_InputHandler->MouseDelta(emath::fvec2(0, 0));
    }
}

//----------------------------------------------------------------------------------

emath::fvec2 GLFWContext::CursorPos() {
    double x, y;
    glfwGetCursorPos(m_Window, &x, &y);
    return emath::fvec2(x, y);
}

//----------------------------------------------------------------------------------



void GLFWContext::GLFW_CursorCallback(GLFWwindow *window, double x, double y) {
    //    GLFWContext* ctx = GetContextFromWindow(window);
    //    Window* w = ((Window*)glfwGetWindowUserPointer(window));
    //    vec2 pos = vec2(x, y);
    //    vec2 delta = pos - w->m_CursorPos;
    //    w->m_CursorPos = pos;
}

void GLFWContext::GLFW_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    GLFWContext *ctx = GetContextFromWindow(window);
    bool state = action == GLFW_PRESS;

    if (!ctx->m_MouseHooked) {
        if (state)
            ctx->HookMouse();
        return;
    }
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnMouseButton(button, state);
}

void GLFWContext::GLFW_ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    GLFWContext *ctx = GetContextFromWindow(window);

    if (!ctx->m_MouseHooked)
        return;

    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnScroll(emath::fvec2(xoffset, yoffset));
}

void GLFWContext::GLFW_KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_UNKNOWN || action == GLFW_REPEAT) {
        return;
    }
    GLFWContext *ctx = GetContextFromWindow(window);

    bool Pressed = (action == GLFW_PRESS);

    switch (key) {
    case GLFW_KEY_PRINT_SCREEN: {
        if (!Pressed)
            return;
        AddLogf(Info, "Got screen shoot key");
        ctx->device->SetCaptureScreenShoot();
        return;
    }
    case GLFW_KEY_ESCAPE:
        if (!Pressed)
            return;
        if (ctx->m_InputHandler) {
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

void GLFWContext::GLFW_CharModeCharCallback(GLFWwindow *window, unsigned int key) {
    GLFWContext *ctx = GetContextFromWindow(window);
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnChar(key, true);
}

void GLFWContext::GLFW_CharModeKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key < GLFW_KEY_ESCAPE) {
        return;
    }
    bool Pressed = (action != GLFW_RELEASE);
    GLFWContext *ctx = GetContextFromWindow(window);
    if (ctx->m_InputHandler)
        ctx->m_InputHandler->OnChar(key, Pressed);
}
#endif
} // namespace MoonGlare::GlfwContext
