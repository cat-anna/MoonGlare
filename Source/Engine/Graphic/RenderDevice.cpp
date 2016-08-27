/*
 * cRenderDevice.cpp
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

#include <Renderer/RenderInput.h>

namespace Graphic {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(cRenderDevice)
RegisterDebugApi(Graphic, &cRenderDevice::RegisterDebugScriptApi, "Graphic");
inline Window* WindowInstance() { return GetRenderDevice()->GetContext().get(); }
RegisterApiDynamicInstance(Window, &WindowInstance, "Window");

//float cRenderDevice::m_gamma = 0;
  
cRenderDevice::cRenderDevice(WindowPtr Context) :
		cRootClass(),
		m_InitThreadId(),
		m_Context(),
		m_CurrentShader(),
		m_CurrentEnvironment(0),
		m_CurrentCamera(0),
		m_DefaultMaterial() {
	SetThisAsInstance();
	m_Context.swap(Context);

	CriticalCheck(glewInit() == GLEW_OK, "Unable to initialize GLEW!");
	AddLog(Debug, "GLEW initialized");
	AddLog(System, "GLEW version: " << (char*)glewGetString(GLEW_VERSION));

	m_InitThreadId = std::this_thread::get_id();
	ReadOpenGLInfo();

	new ShaderManager();
}  
 
cRenderDevice::~cRenderDevice() {
	ShaderManager::DeleteInstance();
}

std::unique_ptr<RenderInput> cRenderDevice::CreateRenderInput() {
	auto ptr = std::make_unique<RenderInput>();
	ptr->Initialize(math::fvec2(GetContextSize()));
	return std::move(ptr);
}

bool cRenderDevice::Initialize() {
	GetShaderMgr()->Initialize();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	//glDisable(GL_FRAMEBUFFER_SRGB);

	glBindTexture(GL_TEXTURE_2D, 0);
	unsigned char texd[] = { 255, 255, 255, };// 0, 255, 0, 0, 0, 255, 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texd);

#ifdef DEBUG
	RegisterDebugCallback();
#endif
	return true;
}

bool cRenderDevice::Finalize() {
	AddLog(Performance, "Frames drawn: " << m_FrameIndex);
	return true;
}

//----------------------------------------------------------------------------------
 
void cRenderDevice::RequestContextManip(LoadQueue::LoadRequestFunc func) {
	if (std::this_thread::get_id() == m_InitThreadId)
		return func();
	m_LoadQueue.QueueRequest(func);
}

void cRenderDevice::DispatchContextManipRequests() {
	if (std::this_thread::get_id() != m_InitThreadId) {
		AddLog(Error, "OpenGL init requests cannot be dispatched from non owning thread!");
		return;
	}
	m_LoadQueue.DispatchAllRequests();
}

//----------------------------------------------------------------------------------

#ifdef DEBUG
void cRenderDevice::RegisterDebugScriptApi(ApiInitializer &api) {
	struct T {
		static void SetWireFrameMode(bool value) {
			if (value) glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			else glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	};
	api
	.addFunction("SetWireFrameMode", &T::SetWireFrameMode)
	//.addVariable("Gamma", &m_gamma, true)
	;
}
#endif

//----------------------------------------------------------------------------------

void cRenderDevice::CheckError() const {
	GLenum err = glGetError();
	if(err == GL_NO_ERROR) return;
	auto ptr = (char*)gluErrorString(err);
	if (!ptr) return;
	AddLog(Error, "OpenGL error: " << ptr);
}

void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
	const char * source_str = "?";
	const char * type_str = "?";
	const char * severity_str = "?";
        
	switch (source) {
	case GL_DEBUG_SOURCE_API_ARB:
		source_str = "OpenGL";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		source_str = "Windows";
		break;	
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		source_str = "Shader Compiler";
		break;	
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		source_str = "Third Party";
		break;	
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		source_str = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER_ARB:
		source_str = "Other";
		break;
	default:
		source_str = "Unknown";
		break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR_ARB:
		type_str = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		type_str = "Deprecated behavior";
		break;	
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		type_str = "Undefined behavior";
		break;	
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		type_str = "Portability";
		break;	
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		type_str = "Performance";
		break;
	case GL_DEBUG_TYPE_OTHER_ARB:
		type_str = "Other";
		break;
	default:
		type_str = "Unknown";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		severity_str = "High";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		severity_str = "Medium";
		break;	
	case GL_DEBUG_SEVERITY_LOW_ARB:
		severity_str = "Low";
		break;	
	default:
		severity_str = "Unknown";
		break;
	}
 
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH_ARB:
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		AddLogf(Error, "OpenGL Error: Source:%s Type:%s ID:%d Severity:%s Message:%s", source_str, type_str, id, severity_str, message);
		return;
	case GL_DEBUG_SEVERITY_LOW_ARB:
	default:
		AddLogf(Warning, "OpenGL Warning: Source:%s Type:%s ID:%d Severity:%s Message:%s", source_str, type_str, id, severity_str, message);
		return;
	}
}
 
void cRenderDevice::RegisterDebugCallback() {
	if (GLEW_ARB_debug_output) {
		glDebugMessageCallbackARB(&DebugCallback, nullptr);

//disable:
//OpenGL Warning: Source:OpenGL Type:Other ID:131185 Severity:Unknown Message:Buffer detailed info: Buffer object X (bound to GL_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations.
		GLuint skip1[] ={ 131185 };
		glDebugMessageControlARB(GL_DEBUG_SOURCE_API_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 1, skip1, GL_FALSE);

		AddLog(Debug, "Debug callback registerd");
	} else
		AddLog(Error, "Debug callback is not supported!");
}

//----------------------------------------------------------------------------------

void cRenderDevice::ReadOpenGLInfo() {
	struct GLInfo_t {
		const char *Name;
		GLenum value;
		int type;
	};
#define _add(NAME, TYPE) { #NAME, NAME, TYPE },
	static const GLInfo_t GLInfo[] = {
		_add(GL_VERSION, 's')
		_add(GL_SHADING_LANGUAGE_VERSION, 's')
		_add(GL_RENDERER, 's')

		_add(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 'f')

		_add(GL_MAX_COLOR_ATTACHMENTS, 'i')
		_add(GL_MAX_TEXTURE_UNITS, 'i')
		_add(GL_MAX_TEXTURE_IMAGE_UNITS, 'i')
		_add(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 'i')
		_add(GL_MAX_TEXTURE_SIZE, 'i')
		_add(GL_MAX_DRAW_BUFFERS, 'i')
		_add(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 'i')
		_add(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 'i')
		
		{},
	};
#undef _add 
	 
/*	GLenum params[] ={
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,//2d integer
		GL_STEREO,//boolean
	};*/

	for (auto *it = GLInfo; it->Name; ++it) {
		switch(it->type) {
		case 's':{
			const char *text = (const char*)glGetString(it->value);
			if (!text)
				AddLog(Warning, "Unable to get OpenGL string " << it->Name);
			else 
				AddLogf(System, "%s = %s", it->Name, text);
			break;
		}
		case 'f':{
			float f;
			glGetFloatv(it->value, &f);
			AddLogf(System, "%s = %f", it->Name, f);
			break;
		}
		case 'i':{
			int i;
			glGetIntegerv(it->value, &i);
			AddLogf(System, "%s = %d", it->Name, i);
			break;
		}
		}
	}
}

//----------------------------------------------------------------------------------

void cRenderDevice::ReadScreenPixels(void *memory, const uvec2 &size, unsigned &glType) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); 
	glReadPixels(0, 0, size[0], size[1], GL_RGB, GL_UNSIGNED_BYTE, memory);
	glType = Flags::fBGR;
}

} // namespace Graphic 
