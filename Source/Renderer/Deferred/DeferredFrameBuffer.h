#pragma once

#include "nfDereferred.h"

namespace MoonGlare::Renderer::Deferred {

class DeferredFrameBuffer {
public:
    DeferredFrameBuffer();
    ~DeferredFrameBuffer();

    struct Buffers {
        enum {
            Position	= SamplerIndex::Position,
            Diffuse		= SamplerIndex::Diffuse,
            Normal		= SamplerIndex::Normal,
            MaxValue,
        };
    };

    void Free();
    bool Reset(const emath::fvec2 &ScreenSize);

    void BeginFrame();

    void BeginGeometryPass();

    //void SetReadBuffer(unsigned b) {
        //glReadBuffer(GL_COLOR_ATTACHMENT0 + b);
    //}

    void BeginLightingPass();
    void BeginFinalPass();
    GLuint m_Textures[Buffers::MaxValue];
    GLuint m_DepthTexture = 0;
    GLuint m_FinalTexture = 0;

    void Bind() const {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
    }
    static void UnBind() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    GLuint m_FrameBuffer = 0;
private:
    bool FreeFrameBuffer();
    bool NewFrameBuffer();
    bool FinishFrameBuffer();
};

} 
