#pragma once

#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

struct Enable {
    GLenum what;
    void Execute() const { glEnable(what); }
};

struct Disable {
    GLenum what;
    void Execute() const { glDisable(what); }
};

} // namespace MoonGlare::Renderer::Commands

#if 0

#include "../CommandQueueBase.h"

//---------------------------------------------------------------------------------------

struct ClearArgument {
	GLbitfield m_Mask;
	static void Execute(const ClearArgument *arg) {
		glClear(arg->m_Mask);
	}
};
using Clear = CommandTemplate<ClearArgument>;

//---------------------------------------------------------------------------------------

struct BlendArgument {
	GLenum m_Equation;
	GLenum m_SFactor;
	GLenum m_DFactor;
	static void Execute(const BlendArgument *arg) {
		glBlendEquation(arg->m_Equation);
		glBlendFunc(arg->m_SFactor, arg->m_DFactor);
	}
};
using Blend = CommandTemplate<BlendArgument>;

//---------------------------------------------------------------------------------------

struct DepthMaskArgument {
	GLboolean m_Value;
	static void Execute(const DepthMaskArgument *arg) {
		glDepthMask(arg->m_Value);
	}
};
using DepthMask = CommandTemplate<DepthMaskArgument>;

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
//---------------------------------------------------------------------------------------

struct CullFaceArgument {
	GLenum m_Mode;
	static void Execute(const CullFaceArgument *arg) {
		glCullFace(arg->m_Mode);
	}
};
using CullFace = CommandTemplate<CullFaceArgument>;

//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

struct StencilFuncArgument {
	GLenum m_Func;
	GLint m_Ref;
	GLuint m_Mask;
	static void Execute(const StencilFuncArgument *arg) {
		glStencilFunc(arg->m_Func, arg->m_Ref, arg->m_Mask);
	}
};
using StencilFunc = CommandTemplate<StencilFuncArgument>;

struct StencilOpSeparateArgument {
	GLenum m_Face;
	GLenum m_sfail;
	GLenum m_dpfail;
	GLenum m_dppass;
	static void Execute(const StencilOpSeparateArgument *arg) {
		glStencilOpSeparate(arg->m_Face, arg->m_sfail, arg->m_dpfail, arg->m_dppass);
	}
};
using StencilOpSeparate = CommandTemplate<StencilOpSeparateArgument>;

} //namespace MoonGlare::Renderer::Commands

#endif
