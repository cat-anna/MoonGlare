/*
	Generated by cppsrc.sh
	On 2014-12-13 13:22:00,37
	by Paweu
*/

#pragma once
#ifndef Window_H
#define Window_H

namespace Graphic {

class Window : public cRootClass {
	GABI_DECLARE_CLASS_NOCREATOR(Window, cRootClass);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
 	Window(bool IsMainWindow = false);
 	~Window();

	static void ProcessWindowSystem();
	static void InitializeWindowSystem();
	static void FinalzeWindowSystem();

	void MakeCurrent() { glfwMakeContextCurrent(m_Window); }
	void SwapBuffers() { glfwSwapBuffers(m_Window); }
	void SetTitle(const char* Title) { glfwSetWindowTitle(m_Window, Title); }

	void ResetViewPort() const { glViewport(0, 0, m_Size[0], m_Size[1]); }

	void GrabMouse() {
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		CursorReset();
		SetMouseHooked(true);
	}
	void ReleaseMouse() {
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		SetMouseHooked(false);
	}

	vec2 CursorPos() {
		double x, y;
		glfwGetCursorPos(m_Window, &x, &y);
		return vec2(x, y);
	}
	vec2 CursorDelta() {
		vec2 pos = CursorPos();
		m_CursorDelta = pos - m_CursorPos;
		m_CursorPos = pos;
		return m_CursorDelta;
	}
	void CursorReset() {
		m_CursorPos = CursorPos();
	}
	   
	struct Flags {
		enum {
			Active					= 0x001,
			MainWindow				= 0x002,

			ConsoleActivated		= 0x010,

			MouseHooked				= 0x100,
			AllowMouseUnhook		= 0x200,
		};								
	};

	unsigned GetRefreshRate() const;

	DefineFlagGetter(m_Flags, Flags::MainWindow, MainWindow);
	DefineFlagGetter(m_Flags, Flags::Active, Active);
	DefineFlagGetter(m_Flags, Flags::ConsoleActivated, ConsoleActivated);
	DefineFlagGetter(m_Flags, Flags::MouseHooked, MouseHooked);
	const uvec2& Size() const { return m_Size; }

	int GetModeCount() const;
	GLFWvidmode GetMode(int index) const;
	GLFWvidmode GetCurrentMode() const;

	void SetPosition(const math::uvec2 &pos);

	static void RegisterDebugScriptApi(::ApiInitializer &api);
	static void RegisterScriptApi(::ApiInitializer &api);
protected:
	void CreateWindow();
	DefineFlagSetter(m_Flags, Flags::Active, Active);
	DefineFlagSetter(m_Flags, Flags::ConsoleActivated, ConsoleActivated);
	DefineFlagSetter(m_Flags, Flags::MouseHooked, MouseHooked);
	DefineFlagSetter(m_Flags, Flags::MainWindow, MainWindow);
	DefineFlag(m_Flags, Flags::AllowMouseUnhook, AllowMouseUnhook);
	void key_callback(int key, bool Pressed);
	void ProcessChar(unsigned  Key);
private:
	GLFWwindow *m_Window;
	unsigned m_Flags;
	vec2 m_CursorPos, m_CursorDelta;
	uvec2 m_Size;

	static void glfw_error_callback(int error, const char* description);
	static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfw_close_callback(GLFWwindow* window);
	static void glfw_focus_callback(GLFWwindow* window, int focus);
	static void glfw_mousepos_callback(GLFWwindow *window, double x, double y);
	static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static void glfw_char_callback(GLFWwindow *window, unsigned int key);
	static bool _GLFWInitialized;
};

} //namespace Graphic 

#endif
