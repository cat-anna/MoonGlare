/*
 * cRenderDevice.h
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */

#ifndef CRENDERDEVICE_H_
#define CRENDERDEVICE_H_

namespace Graphic {

/**
	This class represents (more or less) current state of opengl context
*/
class cRenderDevice : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(cRenderDevice, cRootClass)
public:
	cRenderDevice(WindowPtr Context, Asset::AssetManager *AssetManager);
	virtual ~cRenderDevice();

	mem::aligned_ptr<RenderInput> CreateRenderInput();

	bool Initialize(); 
	bool Finalize();
	void CheckError() const;

	void BeginFrame() { ++m_FrameIndex; }
	void EndFrame() { GetContext()->SwapBuffers(); }
	uint64_t FrameIndex() const { return m_FrameIndex; }

	void ResetViewPort() { GetContext()->ResetViewPort(); }
	void ClearBuffer() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		 
	void BindNullMaterial() { m_DefaultMaterial.Bind(m_CurrentShader); }

	void Bind(const Material &Material) {  Material.Bind(m_CurrentShader);  }
	void Bind(Renderer::VirtualCamera *c) {
		m_CurrentCamera = c; 
		if (c) {
			SetCameraMatrix(c->m_ProjectionMatrix);
		}
	}
	
	void BindEnvironment(const Environment *e) { m_CurrentEnvironment = e; if (e) e->Bind(m_CurrentShader); }

	void Bind(Shader *Shader) { 
		if (!Shader)
			return;
		m_CurrentShader = Shader;
		Shader->Bind();
		if (m_CurrentEnvironment)
			m_CurrentEnvironment->Bind(Shader);
		//CurrentShader()->SetGamma(m_gamma);
		Shader->SetModelMatrix(m_ModelMatrix);
		Shader->SetWorldMatrix(m_WorldMatrix);
		Shader->SetCameraMatrix(m_CameraMatrix);
		if (m_CurrentCamera)
			Shader->SetCameraPos(m_CurrentCamera->m_Position);
	}

	static float m_gamma;

	void SetModelMatrix(const math::mat4 &m) {
		m_ModelMatrix = m;
		m_WorldMatrix = m_CameraMatrix * m_ModelMatrix;
		auto sh = CurrentShader();
		if (!sh) 
			return;
		sh->SetModelMatrix(m_ModelMatrix);
		sh->SetWorldMatrix(m_WorldMatrix);
	}

	void SetCameraMatrix(const math::mat4 &m) { 
		m_CameraMatrix = m; 
		auto sh = CurrentShader();
		if (!sh)
			return;
		sh->SetCameraMatrix(m);
	}

	void SetCameraMatrix(const emath::fmat4 &m) {
		m_CameraMatrix = emath::MathCast<math::mat4>(m);
		auto sh = CurrentShader();
		if (!sh)
			return;
		sh->SetCameraMatrix(m);
	}
	void ResetCameraMatrix() { if(m_CurrentCamera) SetCameraMatrix(m_CurrentCamera->GetProjectionMatrix()); }

	const Environment *CurrentEnvironment() const { return m_CurrentEnvironment; }
	Renderer::VirtualCamera *CurrentCamera() const { return m_CurrentCamera; }
	Shader* CurrentShader() const { return m_CurrentShader; }
	const WindowPtr& GetContext() const { return m_Context; }

	const math::uvec2& GetContextSize() const { return m_Context->Size(); }

	void RequestContextManip(LoadQueue::LoadRequestFunc func);
	void DelayedContextManip(LoadQueue::LoadRequestFunc func) { m_LoadQueue.QueueRequest(func); }
	void DispatchContextManipRequests();

	/** Read pixels from opengl screen */
	void ReadScreenPixels(void *memory, const uvec2 &size, unsigned &glType);

	static void RegisterDebugScriptApi(ApiInitializer &api);
protected:
	volatile uint64_t m_FrameIndex = 0;
	math::mat4 m_CameraMatrix;
	LoadQueue m_LoadQueue;
	std::thread::id m_InitThreadId;

	Asset::AssetManager *m_AssetManager;
	WindowPtr m_Context;
	Shader *m_CurrentShader;
	Renderer::VirtualCamera *m_CurrentCamera;
	const Environment *m_CurrentEnvironment;

	Material m_DefaultMaterial;

	math::mat4 m_ModelMatrix, m_WorldMatrix;
}; 

} // namespace Graphic 
#endif // CRENDERDEVICE_H_ 
