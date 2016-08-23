/*
	Generated by cppsrc.sh
	On 2015-01-18  9:02:18,36
	by Paweu
*/
#include <pch.h>
#include "Graphic.h"

#include "PlaneShadowMap.h"

namespace Graphic {

bool PlaneShadowMap::New() {
	Free();
	NewFrameBuffer();

	//GetRenderDevice()->RequestContextManip([this] {
		m_Size = math::vec2((float)Settings::GetShadowMapSize(::Settings->Graphic.Shadows));
	 
		glGenTextures(1, &m_ShadowTexture); 
		glBindTexture(GL_TEXTURE_2D, m_ShadowTexture);// _TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)m_Size[0], (GLsizei)m_Size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei)m_Size[0], (GLsizei)m_Size[1], 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	  
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ShadowTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowTexture, 0);
	  
//		AddLogf(Error, "shmapid: %d", m_ShadowTexture);

		//bool res =
		FrameBuffer::FinishFrameBuffer();
		FrameBuffer::UnBind();
//	});

	return true;
}

} //namespace Graphic 
