#include <pch.h>
#include <MoonGlare.h>

#include "GeometryShader.h"
#include "LightingShader.h"
#include "PointLightShader.h"
#include "DirectionalLightShader.h"
#include "SpotLightShader.h"

#include "DereferredPipeline.h"

#include <Renderer/RenderInput.h>

namespace Graphic {
namespace Dereferred {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(DereferredPipeline);

DereferredPipeline::DereferredPipeline():
		m_GeometryShader(0),
		m_StencilShader(0),
		m_PointLightShader(0),
		m_Flags(0)
{
}   

DereferredPipeline::~DereferredPipeline() {
} 
   
bool DereferredPipeline::Initialize() { 
	try {

		if (!m_Buffer.Reset()) throw "Unable to initialize render buffers!";
	     
		if (!GetShaderMgr()->GetSpecialShader("Dereferred.Geometry", m_GeometryShader)) throw 0;
		if (!GetShaderMgr()->GetSpecialShader("Dereferred.Light_point", m_PointLightShader)) throw 1;
		if (!GetShaderMgr()->GetSpecialShader("Dereferred.Light_directional", m_DirectionalLightShader)) throw 2;
		if (!GetShaderMgr()->GetSpecialShader("Dereferred.Light_spot", m_SpotLightShader)) throw 3;
		if (!GetShaderMgr()->GetSpecialShader("Dereferred.Stencil", m_StencilShader)) throw 4;
		if (!GetShaderMgr()->GetSpecialShader("Shadows.default", m_ShadowMapShader)) throw 5;

	}
	catch (int idx) {
		AddLogf(Error, "Unable to load shader with index %d", idx);
		return false;
	}
	catch (const char* msg) {
		AddLog(Error, msg);
		return false;
	}
		
	m_Sphere = GetDataMgr()->GetModel("Sphere");
	if (!m_Sphere) {
		AddLog(Warning, "No sphere model! An attempt to render point light will cause failure!");
	} else
		m_Sphere->Initialize();

	m_Cone = GetDataMgr()->GetModel("Cone");
	if (!m_Cone) {
		AddLog(Warning, "No Cone model! An attempt to render spot light will cause failure!");
	} else
		m_Cone->Initialize();

	InitializeDirectionalQuad();

	SetReady(true);
	return true;      
}     
       
bool DereferredPipeline::Finalize() { 
	for (auto &it : m_PlaneShadowMapBuffer)
		it.Free();
	//if (m_Sphere) m_Sphere->Finalize();
	return true;  
}

//--------------------------------------------------------------------------------------
 
bool DereferredPipeline::Execute(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev) {
	if (!IsReady())
		return false;

	auto ri = conf.m_RenderInput.get();

	RenderShadows(ri, dev);
	BeginFrame(dev);
	RenderGeometry(ri, dev);
	RenderLights(ri, dev);

	for (auto *it : conf.CustomDraw) {
		it->DefferedDraw(dev);
	}
	conf.CustomDraw.clear();

	FinalPass(dev.GetContext()->Size());
	EndFrame();
	return true;
}

//--------------------------------------------------------------------------------------
 
void DereferredPipeline::BeginFrame(cRenderDevice& dev) { 
	dev.ResetViewPort();
	dev.ResetCameraMatrix();
	m_Buffer.BeginFrame(); 
} 
 
bool DereferredPipeline::RenderShadows(RenderInput *ri, cRenderDevice& dev) {
	  
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);  

	RenderSpotLightsShadows(ri, dev);

	//float Width = (float)::Settings->Window.Width;
	//float Height = (float)::Settings->Window.Height;
	//glViewport(0, 0, Width, Height);
	//dev.SetD3Mode();

	return true;
}

bool DereferredPipeline::RenderSpotLightsShadows(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_SpotLights.empty()) return true;

	int index = 0;
	for (auto &light : ri->m_SpotLights) {
		if (!light.m_Base.m_Flags.m_CastShadows)
			continue;

		auto &sm = m_PlaneShadowMapBuffer[index];
		if (!sm)
			sm.New();
		++index;

		dev.SetCameraMatrix(light.m_ViewMatrix); 
		sm.BindAndClear();
		dev.Bind(m_ShadowMapShader); 
		m_ShadowMapShader->SetLightPosition(light.m_Position);
		for (auto it : ri->m_RenderList) {
			dev.SetModelMatrix(it.first);
			it.second->DoRender(dev);
		}
	}           
	return true;     
}
 
bool DereferredPipeline::RenderGeometry(RenderInput *ri, cRenderDevice& dev) {
	dev.Bind(m_GeometryShader);
	m_Buffer.BeginGeometryPass(); 
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);  

	//dev.Bind(conf.Camera);
	dev.SetModelMatrix(math::mat4());
	for (auto it : ri->m_RenderList) {
		dev.SetModelMatrix(it.first);
		it.second->DoRender(dev);
	}

	return true;
}

bool DereferredPipeline::RenderLights(RenderInput *ri, cRenderDevice& dev) {
	glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
	RenderPointLights(ri, dev);
	RenderDirectionalLights(ri, dev);
	RenderSpotLights(ri, dev);
	return true;
}

bool DereferredPipeline::RenderPointLights(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_PointLights.empty()) return true;

	StencilTestEnabler Stencil;

	for (auto &light : ri->m_PointLights) {
		//math::mat4 mat; 
		//light.m_SourceTransform.getOpenGLMatrix(&mat[0][0]);
		//mat = glm::scale(mat, math::vec3(light.CalcPointLightInfluenceRadius()));
//shadow pass

//stencil pass
		m_Buffer.BeginStencilPass(); 
		dev.Bind(m_StencilShader);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP); 
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		dev.SetModelMatrix(light.m_PositionMatrix);
		m_Sphere->DoRender(dev);
//light pass	  
		dev.Bind(m_PointLightShader); 
		m_Buffer.BeginLightingPass(); 
		m_PointLightShader->Bind(light); 

		glEnable(GL_BLEND);    
		glBlendEquation(GL_FUNC_ADD); 
		glBlendFunc(GL_ONE, GL_ONE); 
		glDisable(GL_DEPTH_TEST); 
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 	 
		glEnable(GL_CULL_FACE);  
		glCullFace(GL_FRONT);  
		           
		dev.SetModelMatrix(light.m_PositionMatrix);
		m_Sphere->DoRender(dev);  

		glCullFace(GL_BACK);
		glDisable(GL_BLEND); 
		glDisable(GL_CULL_FACE);  
	}

	return true;
}

bool DereferredPipeline::RenderDirectionalLights(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_DirectionalLights.empty()) return true;

	dev.Bind(m_DirectionalLightShader);
	//((Shader*)m_DirectionalLightShader)->Bind();
	m_DirectionalLightShader->SetWorldMatrix(math::mat4());
	m_Buffer.BeginLightingPass(); 
	//dev.SetModelMatrix(math::mat4());
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	m_DirectionalQuad.Bind();
	for (auto &light : ri->m_DirectionalLights) {
		m_DirectionalLightShader->Bind(light);
		m_DirectionalQuad.DrawElements(4, 0, 0, GL_QUADS);
	}
	m_DirectionalQuad.UnBind();
	glDisable(GL_BLEND);
	return true;
}
 
bool DereferredPipeline::RenderSpotLights(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_SpotLights.empty()) return true;

	StencilTestEnabler Stencil;

	int index = 0;
	for (auto &light : ri->m_SpotLights) {
//stencil pass
		m_Buffer.BeginStencilPass(); 
		dev.Bind(m_StencilShader); 
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE); 
		glClear(GL_STENCIL_BUFFER_BIT);
		 
		glStencilFunc(GL_ALWAYS, 0, 0);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP); 
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		dev.SetModelMatrix(light.m_PositionMatrix);
		//m_Cone->DoRender(dev); 
		m_Sphere->DoRender(dev);

		auto &sm = m_PlaneShadowMapBuffer[index];
		++index;

//light pass
		dev.Bind(m_SpotLightShader);
		m_Buffer.BeginLightingPass();   

		m_SpotLightShader->BindShadowMap(sm);
		m_SpotLightShader->SetLightMatrix(light.m_ViewMatrix);
		m_SpotLightShader->Bind(light);

		glEnable(GL_BLEND);      
		glBlendEquation(GL_FUNC_ADD);    
		glBlendFunc(GL_ONE, GL_ONE);  
		glDisable(GL_DEPTH_TEST); 
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 	 
		glEnable(GL_CULL_FACE);    
		glCullFace(GL_FRONT);    

		glActiveTexture(GL_TEXTURE0);   
		glBindTexture(GL_TEXTURE_2D, 0);

		dev.SetModelMatrix(light.m_PositionMatrix);
		//m_Cone->DoRender(dev);
		m_Sphere->DoRender(dev);

		glCullFace(GL_BACK);
		glDisable(GL_BLEND); 
		glDisable(GL_CULL_FACE);   
	}  

	return true;  
}

void DereferredPipeline::EndFrame() {
	GetRenderDevice()->CheckError();
}

void DereferredPipeline::FinalPass(const uvec2 &size) {
	m_Buffer.BeginFinalPass();
    glBlitFramebuffer(0, 0, size[0], size[1], 
                      0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

//--------------------------------------------------------------------------------------

bool DereferredPipeline::InitializeDirectionalQuad() {
	NormalVector Normals{
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
	};
	VertexVector Vertex {
		math::vec3(-1.0f, -1.0f, 0.0f),
		math::vec3( 1.0f, -1.0f, 0.0f),
		math::vec3( 1.0f,  1.0f, 0.0f),
		math::vec3(-1.0f,  1.0f, 0.0f),
	};
	TexCoordVector Texture {
		math::vec2(1.0f, 1.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(0.0f, 0.0f),
		math::vec2(1.0f, 0.0f),
	};
	IndexVector Index {
		0, 1, 2, 3,
	};

	m_DirectionalQuad.Initialize(Vertex, Texture, Normals, Index);

	return true;
}

} //namespace Dereferred
} //namespace Graphic 
