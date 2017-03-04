#include <pch.h>
#include <MoonGlare.h>

#include "DereferredPipeline.h"

#include <Renderer/RenderInput.h>

#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Resources/ResourceManager.h>

namespace Graphic {
namespace Dereferred {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(DereferredPipeline);

DereferredPipeline::DereferredPipeline():
		m_Flags(0)
{
}   

DereferredPipeline::~DereferredPipeline() {
} 
   
bool DereferredPipeline::Initialize(World *world) {
	assert(world);
	m_World = world;

	try {
		if (!m_Buffer.Reset()) throw "Unable to initialize render buffers!";

		auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();

		if (!shres.Load(m_ShaderShadowMapHandle, "ShadowMap")) throw 10;

		if (!shres.Load(m_ShaderLightSpotHandle, "Deferred/LightSpot")) throw 11;
		if (!shres.Load(m_ShaderLightPointHandle, "Deferred/LightPoint")) throw 12;
		if (!shres.Load(m_ShaderLightDirectionalHandle, "Deferred/LightDirectional")) throw 13;
		if (!shres.Load(m_ShaderStencilHandle, "Deferred/Stencil")) throw 15;

		//if (!shres.Load<GeometryShaderDescriptor>(m_ShaderGeometryHandle, "Deferred/Geometry")) throw 15;
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

	FinalPass(dev.GetContext()->Size());
	EndFrame();
	return true;
}

//--------------------------------------------------------------------------------------
 
void DereferredPipeline::BeginFrame(cRenderDevice& dev) { 
	dev.ResetViewPort();
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

		auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();
		auto she = shres.GetExecutor(m_ShaderShadowMapHandle);
		using Uniform = Shaders::ShadowMapShaderDescriptor::Uniform;

		sm.BindAndClear();

		she.Bind();
		she.Set<Uniform::CameraMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_ViewMatrix));
		for (auto it : ri->m_RenderList) {
			she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(it.first));
			it.second->DoRenderMesh(dev);
		}

		using Renderer::RendererConf::CommandQueueID;
		ri->m_CommandQueues[CommandQueueID::DefferedShadow].Execute();
	}           
	return true;     
}
 
bool DereferredPipeline::RenderGeometry(RenderInput *ri, cRenderDevice& dev) {

	//auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();
	//auto she = shres.GetExecutor<GeometryShaderDescriptor>(m_ShaderGeometryHandle);
	//using Uniform = GeometryShaderDescriptor::Uniform;

	//she.Bind();
	//she.Set<Uniform::CameraMatrix>(ri->m_Camera.GetProjectionMatrix());

	dev.Bind(m_GeometryShader);

	m_Buffer.BeginGeometryPass(); 
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);  

	//dev.Bind(conf.Camera);
	//dev.SetModelMatrix(math::mat4());
	for (auto it : ri->m_RenderList) {
		dev.SetModelMatrix(it.first);
		//she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(it.first));
		it.second->DoRender(dev);
	}

	using Renderer::RendererConf::CommandQueueID;
	ri->m_CommandQueues[CommandQueueID::DefferedGeometry].Execute();

	return true;
}

bool DereferredPipeline::RenderLights(RenderInput *ri, cRenderDevice& dev) {
	glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
	RenderDirectionalLights(ri, dev);
	RenderSpotLights(ri, dev);
	RenderPointLights(ri, dev);
	return true;
}

bool DereferredPipeline::RenderPointLights(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_PointLights.empty()) return true;

	StencilTestEnabler Stencil;
	auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();

	for (auto &light : ri->m_PointLights) {
		//math::mat4 mat; 
		//light.m_SourceTransform.getOpenGLMatrix(&mat[0][0]);
		//mat = glm::scale(mat, math::vec3(light.CalcPointLightInfluenceRadius()));
//shadow pass

//stencil pass
		{
			m_Buffer.BeginStencilPass();

			auto she = shres.GetExecutor(m_ShaderStencilHandle);
			using Uniform = StencilLightShaderDescriptor::Uniform;

			she.Bind();
			//dev.Bind(m_StencilShader);
			she.Set<Uniform::CameraMatrix>(ri->m_Camera.GetProjectionMatrix());
			she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_PositionMatrix));

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		//	dev.SetModelMatrix(light.m_PositionMatrix);
			m_Sphere->DoRenderMesh(dev);
		}
//light pass	  
		{
			auto she = shres.GetExecutor(m_ShaderLightPointHandle);
			using Uniform = PointLightShaderDescriptor::Uniform;

			//	dev.Bind(m_PointLightShader); 
			she.Bind();

			auto h = *she.m_HandlePtr;
			auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
			glUniform2fv(glGetUniformLocation(h, "ScreenSize"), 1, &ScreenSize[0]);

			she.Set<Uniform::CameraMatrix>(ri->m_Camera.GetProjectionMatrix());
			she.Set<Uniform::CameraPos>(ri->m_Camera.m_Position);

			m_Buffer.BeginLightingPass();

			//	m_PointLightShader->Bind(light); 

			//	she.Set<Uniform::LightMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_ViewMatrix));

			she.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)light.m_Base.m_Color));
			she.Set<Uniform::AmbientIntensity>(light.m_Base.m_AmbientIntensity);
			she.Set<Uniform::DiffuseIntensity>(light.m_Base.m_DiffuseIntensity);

			//	she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);

			she.Set<Uniform::Position>(emath::MathCast<emath::fvec3>((math::vec3)light.m_Position));

			she.Set<Uniform::AttenuationLinear>(light.m_Attenuation.m_Linear);
			she.Set<Uniform::AttenuationExp>(light.m_Attenuation.m_Exp);
			she.Set<Uniform::AttenuationConstant>(light.m_Attenuation.m_Constant);
			she.Set<Uniform::AttenuationMinThreshold>(light.m_Attenuation.m_Threshold);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_DEPTH_TEST);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);

			//dev.SetModelMatrix(light.m_PositionMatrix);
			she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_PositionMatrix));
			m_Sphere->DoRenderMesh(dev);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);
		}
	}

	return true;
}

bool DereferredPipeline::RenderDirectionalLights(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_DirectionalLights.empty()) return true;

	auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();
	auto she = shres.GetExecutor(m_ShaderLightDirectionalHandle);
	using Uniform = DirectionalLightShaderDescriptor::Uniform;

	she.Bind();

	she.Set<Uniform::CameraPos>(ri->m_Camera.m_Position);

	she.Set<Uniform::CameraMatrix>(emath::MathCast<emath::fmat4>(math::mat4()));
	she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(math::mat4()));

	auto h = *she.m_HandlePtr;
	auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
	glUniform2fv(glGetUniformLocation(h, "ScreenSize"), 1, &ScreenSize[0]);

	m_Buffer.BeginLightingPass(); 
	 
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	m_DirectionalQuad.Bind();
	for (auto &light : ri->m_DirectionalLights) {

		she.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)light.m_Base.m_Color));
		she.Set<Uniform::AmbientIntensity>(light.m_Base.m_AmbientIntensity);
		she.Set<Uniform::DiffuseIntensity>(light.m_Base.m_DiffuseIntensity);
		//	she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);
		//glUniform3fv(m_DirectionLocation, 1, &light.Direction[0]); 

		m_DirectionalQuad.DrawElements(4, 0, 0, GL_QUADS);
	}
	m_DirectionalQuad.UnBind();
	glDisable(GL_BLEND);
	return true;
}
 
bool DereferredPipeline::RenderSpotLights(RenderInput *ri, cRenderDevice& dev) {
	if (ri->m_SpotLights.empty()) return true;

	StencilTestEnabler Stencil;
	auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();

	int index = 0;
	for (auto &light : ri->m_SpotLights) {
//stencil pass
		{
			m_Buffer.BeginStencilPass();
			auto she = shres.GetExecutor(m_ShaderStencilHandle);
			using Uniform = StencilLightShaderDescriptor::Uniform;

			she.Bind();
			//	dev.Bind(m_StencilShader);
			she.Set<Uniform::CameraMatrix>(ri->m_Camera.GetProjectionMatrix());
			she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_PositionMatrix));

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			//dev.SetModelMatrix(light.m_PositionMatrix);
			//m_Cone->DoRender(dev); 
			m_Sphere->DoRenderMesh(dev);
		}
//light pass
		{
			auto &sm = m_PlaneShadowMapBuffer[index];
			++index;

			auto she = shres.GetExecutor(m_ShaderLightSpotHandle);
			using Uniform = SpotLightShaderDescriptor::Uniform;

			she.Bind();

			auto h = *she.m_HandlePtr;
			auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
			glUniform2fv(glGetUniformLocation(h, "ScreenSize"), 1, &ScreenSize[0]);

			she.Set<Uniform::CameraMatrix>(ri->m_Camera.GetProjectionMatrix());
			she.Set<Uniform::CameraPos>(ri->m_Camera.m_Position);

			m_Buffer.BeginLightingPass();

			sm.BindAsTexture(SamplerIndex::PlaneShadow);
			she.Set<Uniform::ShadowMapSize>(emath::MathCast<emath::fvec2>(sm.GetSize()));

			she.Set<Uniform::LightMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_ViewMatrix));

			she.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)light.m_Base.m_Color));
			she.Set<Uniform::AmbientIntensity>(light.m_Base.m_AmbientIntensity);
			she.Set<Uniform::DiffuseIntensity>(light.m_Base.m_DiffuseIntensity);

			she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);

			she.Set<Uniform::Position>(emath::MathCast<emath::fvec3>((math::vec3)light.m_Position));
			she.Set<Uniform::Direction>(emath::MathCast<emath::fvec3>((math::vec3)light.m_Direction));

			she.Set<Uniform::CutOff>(light.m_CutOff);

			she.Set<Uniform::AttenuationLinear>(light.m_Attenuation.m_Linear);
			she.Set<Uniform::AttenuationExp>(light.m_Attenuation.m_Exp);
			she.Set<Uniform::AttenuationConstant>(light.m_Attenuation.m_Constant);
			she.Set<Uniform::AttenuationMinThreshold>(light.m_Attenuation.m_Threshold);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_DEPTH_TEST);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);

			//dev.SetModelMatrix(light.m_PositionMatrix);
			she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_PositionMatrix));

			//m_Cone->DoRender(dev);
			m_Sphere->DoRenderMesh(dev);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);
		}
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
