/*
  * Generated by cppsrc.sh
  * On 2017-02-11 20:30:17,81
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "TextureRenderTask.h"

namespace MoonGlare::Renderer {


TextureRenderTask::TextureRenderTask() {
	m_Framebuffer = 0;
}

TextureRenderTask::~TextureRenderTask() {
}

//---------------------------------------------------------------------------------------

bool TextureRenderTask::Initialize() {
	m_CommandQueue.MemZero();
	m_CommandQueue.ClearAllocation();

	glGenFramebuffers(1, &m_Framebuffer); 

	return true;
}

bool TextureRenderTask::Finalize() {
	glDeleteFramebuffers(1, &m_Framebuffer);

	return true;
}

void TextureRenderTask::Execute() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, m_TargetTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Size[0], m_Size[1], 0, GL_RGB, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Size[0], m_Size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//	GLuint depthrenderbuffer;
//	glGenRenderbuffers(1, &depthrenderbuffer);
//	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	//glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TargetTexture, 0);
//'	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TargetTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TargetTexture, 0);

	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer);
	static const GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0, };
	glDrawBuffers(1, DrawBuffers); 

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		AddLogf(Error, "Framebuffer error!");
		return;
	}	 

	glViewport(0, 0, m_Size[0], m_Size[1]); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_CommandQueue.Execute();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, m_TargetTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//---------------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer 

