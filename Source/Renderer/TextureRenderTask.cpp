/*
  * Generated by cppsrc.sh
  * On 2017-02-11 20:30:17,81
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "TextureRenderTask.h"

#include "Frame.h"
#include "Resources/ResourceManager.h"

#include "Commands/OpenGL/FramebufferCommands.h"
#include "Commands/OpenGL/ControllCommands.h"
#include "Commands/OpenGL/TextureCommands.h"

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

void TextureRenderTask::Begin() {
	using namespace Commands;

	TextureHandle *htable = m_Frame->GetResourceManager()->GetTextureAllocator().GetHandleArrayBase();

	m_CommandQueue.MakeCommand<FramebufferDrawBind>(m_Framebuffer);
	m_CommandQueue.MakeCommand<Disable>((GLenum)GL_DEPTH_TEST);
	m_CommandQueue.MakeCommand<Disable>((GLenum)GL_BLEND);

	auto bind = m_CommandQueue.PushCommand<Texture2DResourceBind>();
	bind->m_HandleArray = htable;
	bind->m_Handle = m_TargetTexture;

	auto teximg = m_CommandQueue.PushCommand<Texture2DImage>();
	teximg->m_Format = GL_RGBA;
	teximg->m_InternalFormat = GL_RGBA;
	teximg->m_Type = GL_UNSIGNED_BYTE;
	teximg->m_Pixels = nullptr;
	teximg->m_Size[0] = m_Size[0];
	teximg->m_Size[1] = m_Size[1];

	m_CommandQueue.MakeCommand<Texture2DParameterInt>((GLenum)GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_CommandQueue.MakeCommand<Texture2DParameterInt>((GLenum)GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_CommandQueue.MakeCommand<Texture2DParameterInt>((GLenum)GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_CommandQueue.MakeCommand<Texture2DParameterInt>((GLenum)GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	auto sfb = m_CommandQueue.MakeCommand<SetFramebufferDrawTexture>();
	sfb->m_HandleArray = htable;
	sfb->m_Handle = m_TargetTexture;
	sfb->m_ColorAttachment = GL_COLOR_ATTACHMENT0;

	static const GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0, };
	m_CommandQueue.MakeCommand<SetDrawBuffers>(1, DrawBuffers);

	m_CommandQueue.MakeCommand<CheckDrawFramebuffer>("TextureRenderTask");
	
	m_CommandQueue.MakeCommand<SetViewport>(0, 0, m_Size[0], m_Size[1]);
	m_CommandQueue.MakeCommand<Clear>((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void TextureRenderTask::End() {
	TextureHandle *htable = m_Frame->GetResourceManager()->GetTextureAllocator().GetHandleArrayBase();

	m_CommandQueue.MakeCommand<Commands::FramebufferDrawBind>((FramebufferHandle)0);

	auto bind = m_CommandQueue.PushCommand<Commands::Texture2DResourceBind>();
	bind->m_HandleArray = htable;
	bind->m_Handle = m_TargetTexture;

	m_CommandQueue.MakeCommand<Commands::Texture2DParameterInt>((GLenum)GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_CommandQueue.MakeCommand<Commands::Texture2DParameterInt>((GLenum)GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void TextureRenderTask::SetTarget(TextureResourceHandle &handle, emath::ivec2 Size)  {
	if (handle.m_TmpGuard == 0) { 
		if (!m_Frame->GetResourceManager()->GetTextureAllocator().Allocate(m_CommandQueue, handle)) {
			AddLogf(Warning, "Texture allocation failed!");
		}
	}
	m_TargetTexture = handle; 
	m_Size = Size;
}

#if 0
void TextureRenderTask::Execute() {
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Framebuffer);

	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);

	//auto TargetTexture = 0;

	//glBindTexture(GL_TEXTURE_2D, TargetTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Size[0], m_Size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

/*
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TargetTexture, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TargetTexture, 0);
*/
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TargetTexture, 0);

	//static const GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0, };
	//glDrawBuffers(1, DrawBuffers); 

	//if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	//	AddLogf(Error, "Framebuffer error!");
	//	return;
	//}	 

	//glViewport(0, 0, m_Size[0], m_Size[1]); 
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_CommandQueue.Execute();

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//glBindTexture(GL_TEXTURE_2D, TargetTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
#endif

//---------------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer 

