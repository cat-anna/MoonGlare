#pragma once

#include "../CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands{

struct EnableArgument {
	GLenum m_What;
	static void Execute(const EnableArgument *arg) {
		glEnable(arg->m_What);
	}
};
using Enable = CommandTemplate<EnableArgument>;

struct DisableArgument {
	GLenum m_What;
	static void Execute(const DisableArgument *arg) {
		glDisable(arg->m_What);
	}
};
using Disable = CommandTemplate<DisableArgument>;

//---------------------------------------------------------------------------------------

struct ClearArgument {
	GLbitfield m_Mask;
	static void Execute(const ClearArgument *arg) {
		glClear(arg->m_Mask);
	}
};
using Clear = CommandTemplate<ClearArgument>;

//---------------------------------------------------------------------------------------

struct SetViewportArgument {
	GLint m_X;
	GLint m_Y;
	GLsizei m_Width;
	GLsizei m_Height;
	static void Execute(const SetViewportArgument *arg) {
		glViewport(arg->m_X, arg->m_Y, arg->m_Width, arg->m_Height);
	}
};
using SetViewport = CommandTemplate<SetViewportArgument>;

//---------------------------------------------------------------------------------------

struct EnterWireFrameModeArgument {
	static void Execute(const EnterWireFrameModeArgument *arg) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
};
using EnterWireFrameMode = CommandTemplate<EnterWireFrameModeArgument>;

struct LeaveWireFrameModeArgument {
	static void Execute(const LeaveWireFrameModeArgument *arg) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
};
using LeaveWireFrameMode = CommandTemplate<LeaveWireFrameModeArgument>;

} //namespace MoonGlare::Renderer::Commands
