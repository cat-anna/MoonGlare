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
 	PlaneShadowMap();
 	~PlaneShadowMap();

	bool New();
	bool Free();

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
protected:
	GLuint m_ShadowTexture = 0;
	math::vec2 m_Size;
};

} //namespace Graphic 

#endif
