/*
	Generated by cppsrc.sh
	On 2014-12-13 13:22:00,37
	by Paweu
*/

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iApplication.h>
#include <Engine/Core/Console.h>

namespace Graphic {

static string DumpGLFWMode(const GLFWvidmode* mode) {
	char buf[128];
	sprintf(buf, "%dx%d %dhz  bits: %dx%dx%d  ratio:%f", mode->width, mode->height, mode->refreshRate, mode->redBits, mode->blueBits, mode->greenBits, (float)mode->width/(float)mode->height);
	return buf;
}

static bool IsModeSuggested(const GLFWvidmode* mode) {
	if (mode->blueBits != 8 || mode->greenBits != 8 || mode->redBits != 8)
		return false;
	if (mode->refreshRate != 60)
		return false;
	if (mode->width < 1024)
		return false;
	float ratio = (float)mode->width / (float)mode->height;
	if (!((ratio >= 1.3f && ratio <= 1.4f) || (ratio >= 1.7f && ratio <= 1.8f)))
		return false;
	return true;
}

//-------------------------------------------------------------------------------------------------

GABI_IMPLEMENT_CLASS_NOCREATOR(Window)
RegisterApiDerivedClass(Window, &Window::RegisterScriptApi);
RegisterDebugApi(DbgWindow, &Window::RegisterDebugScriptApi, "Graphic");

bool Window::_GLFWInitialized = false;

Window::Window(bool IsMainWindow):
		BaseClass(),
		m_Window(0), 
		m_Flags(0), 
		m_CursorPos(0), 
		m_CursorDelta(0), 
		m_Size(0) {
	InitializeWindowSystem();
	THROW_ASSERT(_GLFWInitialized, "GLFW is not initialized. Cannot create window!");
	CreateWindow();
	//SetEnableConsole(::Settings->Window.EnableConsole);
	SetAllowMouseUnhook(DEBUG_TRUE);
	SetMainWindow(IsMainWindow);
}

Window::~Window() {
	glfwDestroyWindow(m_Window);
}

//-------------------------------------------------------------------------------------------------

void Window::RegisterDebugScriptApi(::ApiInitializer &api) {
	struct funcs {
#ifdef DEBUG_SCRIPTAPI
		static void DumpGraphicModes() {
			int count;
			GLFWmonitor** monitors = glfwGetMonitors(&count);
			AddLog(Hint, "Detected Monitors: " << count);
			for (int i = 0; i < count; ++i) {
				int modecount;
				int widthMM, heightMM;
				GLFWmonitor* monitor = monitors[i];
				glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);
				AddLogf(Hint, "Monitor %d: %s %s", i, glfwGetMonitorName(monitor), (glfwGetPrimaryMonitor() == monitor?"PRIMARY":""));
				AddLogf(Hint, "Physical size: %dx%d mm ", widthMM, heightMM);
				AddLog(Hint, "Current mode: " << DumpGLFWMode(glfwGetVideoMode(monitor)));
				const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modecount);
				AddLogf(Hint, "Available modes: ");
				for (int j = 0; j < modecount; ++j) {
					AddLog(Hint, j << ". -> " << DumpGLFWMode(modes + j));
				}
				AddLogf(Hint, "Suggested modes: ");
				for (int j = 0; j < modecount; ++j) {
					auto *mode = modes + j;
					if (!IsModeSuggested(mode))
						continue;
					
					AddLog(Hint, j << ". -> " << DumpGLFWMode(mode));
				}
			}
		}
#endif
	};

	api
#ifdef DEBUG_SCRIPTAPI
		.addFunction("DumpGraphicModes", &funcs::DumpGraphicModes)
#endif
		;
}

void Window::RegisterScriptApi(::ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cWindow")
		.addFunction("SetTitle", &ThisClass::SetTitle)

		.addFunction("GetMode", &ThisClass::GetMode)
		.addFunction("GetModeCount", &ThisClass::GetModeCount)
		.addFunction("GetCurrentMode", &ThisClass::GetCurrentMode)
#ifdef DEBUG
		.addFunction("GrabMouse", &ThisClass::GrabMouse)
		.addFunction("ReleaseMouse", &ThisClass::ReleaseMouse)
		.addFunction("SetPosition", Utils::Template::DynamicArgumentConvert<ThisClass, math::uvec2, &ThisClass::SetPosition, int, int>::get())
#endif
	.endClass()
	.beginClass<GLFWvidmode>("cGLFWvidmode")
		.addData("Height", &GLFWvidmode::height, false)
		.addData("Width", &GLFWvidmode::width, false)
		.addData("RefreshRate", &GLFWvidmode::refreshRate, false)
	.endClass()
	; 
}

//-------------------------------------------------------------------------------------------------

int Window::GetModeCount() const {
	auto monitor = glfwGetWindowMonitor(m_Window);
	if (!monitor)
		monitor = glfwGetPrimaryMonitor();

	int modecount;
	const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modecount);
	int count = 0;
	for (int j = 0; j < modecount; ++j) {
		if (!IsModeSuggested(modes + j))
			continue;
		++count;
	}
	return count;
}

GLFWvidmode Window::GetMode(int index) const {
	auto monitor = glfwGetWindowMonitor(m_Window);
	if (!monitor)
		monitor = glfwGetPrimaryMonitor();

	int modecount;
	const GLFWvidmode* modes = glfwGetVideoModes(monitor, &modecount);
	for (int j = 0; j < modecount; ++j) {
		if (!IsModeSuggested(modes + j))
			continue;
		if (index <= 0)
			return modes[j];
		--index;
	}
	return GLFWvidmode{};
}

GLFWvidmode Window::GetCurrentMode() const {
	return GLFWvidmode{
		(int)m_Size[0],
		(int)m_Size[1],
		8, 8, 8,
		(int)GetRefreshRate()
	};
}

void Window::SetPosition(const math::uvec2 &pos) {
	glfwSetWindowPos(m_Window, pos[0], pos[1]);
}

//-------------------------------------------------------------------------------------------------

void Window::CreateWindow() {
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	//glfwWindowHint(GLFW_SRGB_CAPABLE, GL_FALSE);

	//auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
	
	unsigned w = Renderer::GraphicSettings::Width::get();
	unsigned h = Renderer::GraphicSettings::Height::get();
	GLFWmonitor *monitor = nullptr;

	if (Renderer::GraphicSettings::FullScreen::get()) {
		monitor = glfwGetPrimaryMonitor();
	}

	if (!w || !h) {
		auto mon = glfwGetPrimaryMonitor();
		auto mode = glfwGetVideoMode(mon);
		w = mode->width; h = mode->height;
		Renderer::GraphicSettings::Width::set(w);
		Renderer::GraphicSettings::Height::set(h);
	}

	m_Size = uvec2(w, h);
	m_Window = glfwCreateWindow(w, h, "MoonGlare engine window", monitor, 0);
	CriticalCheck(m_Window, "Unable to create new window!");

	MakeCurrent();
	glfwSwapInterval(0);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, glfw_key_callback);
	glfwSetWindowCloseCallback(m_Window, glfw_close_callback);
	glfwSetMouseButtonCallback(m_Window, glfwMouseButtonCallback);
	glfwSetWindowFocusCallback(m_Window, glfw_focus_callback);
	//glfwSetCursorPosCallback(m_Window, glfw_mousepos_callback);

	AddLogf(Debug, "Created window %dx%d", w, h);
}

void Window::key_callback(int key, bool Pressed) {
	if (IsMainWindow() && IsConsoleActivated()){
		if (!Pressed) return;
		ProcessChar(key | 0x10000000);
		return;
	}
	switch (key) {
	case GLFW_KEY_ESCAPE:
		if (!Pressed) return;
		if (TestFlags(m_Flags, Flags::AllowMouseUnhook | Flags::MouseHooked)) {
			ReleaseMouse();
			::Core::GetInput()->ClearMouseDelta();
			return;
		}
		::Core::GetEngine()->HandleEscapeKey();
		return;
	case GLFW_KEY_GRAVE_ACCENT://` - 0xC0
		if (!Pressed) return;
		if (!::Settings->Engine.EnableConsole)
			return;
		SetConsoleActivated(true);
		glfwSetCharCallback(m_Window, glfw_char_callback);
		AddLog(Debug, "Console activated");
		return;
	case GLFW_KEY_PRINT_SCREEN:
		if (!Pressed) return;
		::Core::GetEngine()->CaptureScreenShot();
		return;
	default:
		::Core::GetInput()->KeyChange((unsigned)key, Pressed);
		return;
	}
	//	AddLog(Hint, "Key: " << Key << "  hex: 0x" << std::hex << (unsigned)Key);
}

void Window::ProcessChar(unsigned Key) {
	if (Key & 0x10000000){
		Key &= ~0x10000000;
		switch ((WindowInput::Key)Key) {
		case WindowInput::Key::Escape:
		case WindowInput::Key::GraveAccent:
//			Core::GetEngine()->GetConsole()->GetInputLine().Clear();
			SetConsoleActivated(false);
			glfwSetCharCallback(m_Window, 0);
			AddLog(Debug, "Console deactivated"); 
			MoonGlare::Core::Console::Instance()->CancelInput();
			return;
		//case Keys::Enter:
		//case Keys::ArrowUp:
		//case Keys::ArrowDown:
		//case Keys::ArrowLeft:
		//case Keys::ArrowRight:
		//case Keys::Delete:
		//case Keys::Backspace:
		default:
			if (Key < 256) return; //skip glfw non functional key codes
			return MoonGlare::Core::Console::Instance()->ProcessInput(Key);
		}
	}
	switch (Key) {
	case GLFW_KEY_GRAVE_ACCENT:
		return;
	}
	return MoonGlare::Core::Console::Instance()->ProcessInput(Key);
}

// handling GLFW

void Window::InitializeWindowSystem() {
	if (_GLFWInitialized) return;
	glfwSetErrorCallback(&glfw_error_callback);

	CriticalCheck(glfwInit(), "Unable to initialize GLFW!");
	AddLog(Debug, "GLFW initialized");
	AddLog(SysInfo, "GLFW version: " << glfwGetVersionString());

	_GLFWInitialized = true;

	auto monitor = glfwGetPrimaryMonitor();
	AddLogf(SysInfo, "Primary monitor: %s", glfwGetMonitorName(monitor));
	AddLog(SysInfo, "Current mode: " << DumpGLFWMode(glfwGetVideoMode(monitor)));
}

void Window::FinalzeWindowSystem() {
	if (!_GLFWInitialized) return;
	glfwTerminate();
	_GLFWInitialized = false;
	AddLog(Debug, "GLFW finalized");
}

void Window::ProcessWindowSystem() {
	glfwPollEvents(); 
}

void Window::glfw_mousepos_callback(GLFWwindow *window, double x, double y) {
	Window* w = ((Window*)glfwGetWindowUserPointer(window));
	vec2 pos = vec2(x, y);
	vec2 delta = pos - w->m_CursorPos;
	w->m_CursorPos = pos;
}

void Window::glfw_error_callback(int error, const char* description) {
	AddLogf(Error, "GLFW error: code='%d' descr='%s'", error, description);
}

void Window::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (action == GLFW_REPEAT) {
		//AddLogf(Debug, "glfw repeat: %d", key);
		return;
	}
	Window* w = ((Window*)glfwGetWindowUserPointer(window));
	//AddLog(Hint, "key: " << key << "  hex: 0x" << std::hex << (unsigned)key << " act:" << action);
	w->key_callback(key, action == GLFW_PRESS);
}

void Window::glfw_char_callback(GLFWwindow* window, unsigned int key) {
	Window* w = ((Window*)glfwGetWindowUserPointer(window));
	//AddLog(Hint, "char: " << key << "  hex: 0x" << std::hex << (unsigned)key);
	//if (w->IsEnableConsole())
		w->ProcessChar(key);
}

void Window::glfw_close_callback(GLFWwindow* window){
	Window* w = ((Window*)glfwGetWindowUserPointer(window));
	if (w->IsMainWindow())
		GetApplication()->Exit();
}

void Window::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int rawmods){
	Window* w = ((Window*)glfwGetWindowUserPointer(window));
	if (!w->IsMouseHooked()) {
		if (action != GLFW_PRESS) return;
		w->GrabMouse();
		return;
	}

	auto MouseBtn = static_cast<WindowInput::MouseButton>(button);
	WindowInput::ModsStatus mods{ (unsigned)rawmods };

	if (action == GLFW_PRESS)
		::Core::Input::MouseDownEvent(MouseBtn, mods);
	else
		::Core::Input::MouseUpEvent(MouseBtn, mods);
}

void Window::glfw_focus_callback(GLFWwindow* window, int focus) {
	Window* w = ((Window*)glfwGetWindowUserPointer(window));

	//if (!(app->m_Flags & AppFlag_AllowDeactivation))	return;
	bool inactive = !(focus == GL_FALSE);
	AddLog(Debug, "Window changed focus state to " << std::boolalpha << inactive);
	w->SetActive(inactive);
	if (!w->IsMainWindow()) return;
	if (!inactive) {
		//w->ReleaseMouse();
		GetApplication()->OnDeactivate();
	} else {
		GetApplication()->OnActivate();
		//w->GrabMouse();
	}
}

//-------------------------------------------------------------------------------------------------

unsigned Window::GetRefreshRate() const {
	auto monitor = glfwGetWindowMonitor(m_Window);
	if (!monitor)
		monitor = glfwGetPrimaryMonitor();
	return glfwGetVideoMode(monitor)->refreshRate;
}

} //namespace Graphic 
