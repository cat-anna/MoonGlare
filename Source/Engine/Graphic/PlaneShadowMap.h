/*
    Generated by cppsrc.sh
    On 2015-01-18  9:02:18,36
    by Paweu
*/

#pragma once
#ifndef PlaneShadowMap_H
#define PlaneShadowMap_H

namespace Graphic {

class PlaneShadowMap : public FrameBuffer {
public:
    PlaneShadowMap() {
        m_ShadowTexture = 0;
    }
    ~PlaneShadowMap() { 
        Free();
    }

    bool New();
    bool Free() {
        FreeFrameBuffer();
        if (m_ShadowTexture != 0)
            GetRenderDevice()->RequestContextManip([this] {
                glDeleteTextures(1, &m_ShadowTexture);
                m_ShadowTexture = 0;
            });
        return true;
    }

    bool Valid() const {
        return m_ShadowTexture != 0;
    }
    operator bool() const { return Valid(); }

    void BindAndClear() {
        glViewport(0, 0, static_cast<int>(m_Size[0]), static_cast<int>(m_Size[1]));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }

    void BindAsTexture(GLenum TextureIndex) const {
        glActiveTexture(GL_TEXTURE0 + TextureIndex);
        glBindTexture(GL_TEXTURE_2D, m_ShadowTexture);
    }

    const math::vec2& GetSize() const { return m_Size; }
    GLuint Handle() { return m_ShadowTexture; }
    GLuint FBHandle() { return m_FrameBuffer; }
protected:
    GLuint m_ShadowTexture;
    math::vec2 m_Size;
};

} //namespace Graphic 

#endif
