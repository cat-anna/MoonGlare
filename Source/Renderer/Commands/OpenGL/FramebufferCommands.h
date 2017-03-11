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


struct FramebufferReadBindArgument {
    Device::FramebufferHandle m_handle;
    static void Execute(const FramebufferReadBindArgument *arg) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, arg->m_handle);
    }
};
using FramebufferReadBind = CommandTemplate<FramebufferReadBindArgument>;

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

struct SetDrawBufferArgument {
    GLenum m_Value;
    static void Execute(const SetDrawBufferArgument *arg) {
        glDrawBuffer(arg->m_Value);
    }
};
using SetDrawBuffer = CommandTemplate<SetDrawBufferArgument>;

//---------------------------------------------------------------------------------------

struct SetReadBufferArgument {
    GLenum m_Value;
    static void Execute(const SetReadBufferArgument *arg) {
        glReadBuffer(arg->m_Value);
    }
};
using SetReadBuffer = CommandTemplate<SetReadBufferArgument>;

//---------------------------------------------------------------------------------------

struct BlitFramebufferArgument {
    GLint m_srcX0; 
    GLint m_srcY0; 
    GLint m_srcX1; 
    GLint m_srcY1; 
    GLint m_dstX0; 
    GLint m_dstY0;
    GLint m_dstX1; 
    GLint m_dstY1; 
    GLbitfield m_mask; 
    GLenum m_filter;
    void Run() {
        glBlitFramebuffer(m_srcX0, m_srcY0, m_srcX1, m_srcY1, m_dstX0, m_dstY0, m_dstX1, m_dstY1, m_mask, m_filter);
    }
};
using BlitFramebuffer = RunnableCommandTemplate<BlitFramebufferArgument>;

} //namespace MoonGlare::Renderer::Commands
