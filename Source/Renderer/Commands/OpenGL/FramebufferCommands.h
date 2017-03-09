#pragma once

#include "../CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands{

struct CheckDrawFramebufferArgument {
	const char* m_UserId;
	static void Execute(const CheckDrawFramebufferArgument *arg) {
		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			AddLogf(Error, "Framebuffer error! user: %s", arg->m_UserId);
		}
	}
};
using CheckDrawFramebuffer = CommandTemplate<CheckDrawFramebufferArgument>;

//---------------------------------------------------------------------------------------

struct FramebufferBindArgument {
	Device::FramebufferHandle m_handle;
	static void Execute(const FramebufferBindArgument *arg) {
		glBindFramebuffer(GL_FRAMEBUFFER, arg->m_handle);
	}
};
using FramebufferBind = CommandTemplate<FramebufferBindArgument>;

struct FramebufferDrawBindArgument {
	Device::FramebufferHandle m_handle;
	static void Execute(const FramebufferDrawBindArgument *arg) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, arg->m_handle);
	}
};
using FramebufferDrawBind = CommandTemplate<FramebufferDrawBindArgument>;

//---------------------------------------------------------------------------------------

struct SetFramebufferDrawTextureArgument : public TextureCommandBase {
	GLenum m_ColorAttachment;
	static void Execute(const SetFramebufferDrawTextureArgument *arg) {
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, arg->m_ColorAttachment, arg->m_HandleArray[arg->m_Handle.m_Index], 0);
	}
};
using SetFramebufferDrawTexture = CommandTemplate<SetFramebufferDrawTextureArgument>;

//---------------------------------------------------------------------------------------

struct SetDrawBuffersArgument {
	GLsizei m_Count;
	const GLenum* m_BufferTable;
	static void Execute(const SetDrawBuffersArgument *arg) {
		glDrawBuffers(arg->m_Count, arg->m_BufferTable);
	}
};
using SetDrawBuffers = CommandTemplate<SetDrawBuffersArgument>;

} //namespace MoonGlare::Renderer::Commands
