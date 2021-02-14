#pragma once

#include "nfDereferred.h"
#include <gl/glew.h>
#include <math/EigenMath.h>

namespace MoonGlare::Renderer::Deferred {

class DeferredFrameBuffer {
public:
    DeferredFrameBuffer();
    ~DeferredFrameBuffer();

    struct Buffers {
        enum {
            Final = 0,
            Position = 1,
            Diffuse = 2,
            Normal = 3,
            Specular = 4,
            Emissive = 5,
            MaxValue,
        };
    };

    void Free();
    bool Reset(const emath::fvec2 &ScreenSize);

    void BeginFrame();

    void BeginGeometryPass();

    // void SetReadBuffer(unsigned b) {
    // glReadBuffer(GL_COLOR_ATTACHMENT0 + b);
    //}

    void BeginLightingPass();
    void BeginFinalPass();
    GLuint m_Textures[Buffers::MaxValue];
    GLuint m_DepthTexture = 0;

    void Bind() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer); }
    static void UnBind() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); }
    GLuint m_FrameBuffer = 0;

private:
    bool FreeFrameBuffer();
    bool NewFrameBuffer();
    bool FinishFrameBuffer();
};

} // namespace MoonGlare::Renderer::Deferred
