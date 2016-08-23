/*
	Generated by cppsrc.sh
	On 2015-01-17 22:35:49,94
	by Paweu
*/
#include <pch.h>
#include "Graphic.h"
//#include <EngineHeader.h>
#include "FrameBuffer.h"

namespace Graphic {

FrameBuffer::FrameBuffer() {
}

FrameBuffer::~FrameBuffer() {
	FreeFrameBuffer();
}

bool FrameBuffer::FreeFrameBuffer() {
	if(m_FrameBuffer != 0)
		GetRenderDevice()->RequestContextManip([this] {
			glDeleteFramebuffers(1, &m_FrameBuffer);
			m_FrameBuffer = 0;
		});
	return true;
}

bool FrameBuffer::NewFrameBuffer() {
	if (m_FrameBuffer) FreeFrameBuffer();
	GetRenderDevice()->RequestContextManip([this] {
		glGenFramebuffers(1, &m_FrameBuffer);    
		Bind();
	});
	return true;
}

bool FrameBuffer::FinishFrameBuffer() {
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        AddLogf(Error, "FB error, status: 0x%x\n", Status);
        return false;
    }
	return true;
}

} //namespace Graphic 
