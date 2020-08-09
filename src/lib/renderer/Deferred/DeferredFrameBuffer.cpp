#include "DeferredFrameBuffer.h"
#include <orbit_logger.h>

namespace MoonGlare::Renderer::Deferred {

DeferredFrameBuffer::DeferredFrameBuffer() { memset(m_Textures, 0, sizeof(m_Textures)); }

DeferredFrameBuffer::~DeferredFrameBuffer() { Free(); }

void DeferredFrameBuffer::Free() {
    FreeFrameBuffer();

    if (m_Textures[0]) {
        glDeleteTextures(Buffers::MaxValue, m_Textures);
    }

    if (m_DepthTexture) {
        glDeleteTextures(1, &m_DepthTexture);
    }

    // if (m_FinalTexture) {
    //    glDeleteTextures(1, &m_FinalTexture);
    //}
}

bool DeferredFrameBuffer::FreeFrameBuffer() {
    if (m_FrameBuffer != 0)
        glDeleteFramebuffers(1, &m_FrameBuffer);
    m_FrameBuffer = 0;
    return true;
}

bool DeferredFrameBuffer::NewFrameBuffer() {
    if (m_FrameBuffer)
        FreeFrameBuffer();
    glGenFramebuffers(1, &m_FrameBuffer);
    Bind();
    return true;
}

bool DeferredFrameBuffer::FinishFrameBuffer() {
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        AddLogf(Error, "FB error, status: 0x%x\n", Status);
        return false;
    }
    return true;
}

void DeferredFrameBuffer::BeginFrame() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT4);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DeferredFrameBuffer::BeginGeometryPass() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
    static const GLenum DrawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
    };
    glDrawBuffers(4, DrawBuffers);
}

void DeferredFrameBuffer::BeginLightingPass() {
    glDrawBuffer(GL_COLOR_ATTACHMENT4);
    for (unsigned int i = 0; i < Buffers::MaxValue; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_Textures[i]);
    }
}

void DeferredFrameBuffer::BeginFinalPass() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT4);
}

bool DeferredFrameBuffer::Reset(const emath::fvec2 &ScreenSize) {
    Free();
    NewFrameBuffer();
    auto s = ScreenSize;

    glGenTextures(Buffers::MaxValue, m_Textures);
    glGenTextures(1, &m_DepthTexture);
    // glGenTextures(1, &m_FinalTexture);

    for (unsigned int i = 0; i < Buffers::MaxValue; i++) {
        glBindTexture(GL_TEXTURE_2D, m_Textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)s[0], (GLsizei)s[1], 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_Textures[i], 0);
    }

    // depth
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, (GLsizei)s[0], (GLsizei)s[1], 0, GL_DEPTH_STENCIL,
                 GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

    // final
    // glBindTexture(GL_TEXTURE_2D, m_FinalTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)s[0], (GLsizei)s[1], 0, GL_RGBA, GL_FLOAT, NULL);
    // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Buffers::MaxValue, GL_TEXTURE_2D,
    // m_FinalTexture, 0);

    FinishFrameBuffer();
    UnBind();

    return true;
}

} // namespace MoonGlare::Renderer::Deferred
