#include <pch.h>
#include <MoonGlare.h>

namespace Graphic {
namespace Dereferred {

GABI_IMPLEMENT_CLASS_NOCREATOR(DereferredPipeline);

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
	//if (m_Sphere) m_Sphere->Finalize();
	return true;  
}

//--------------------------------------------------------------------------------------
 
bool DereferredPipeline::Execute(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev) {
	if (!IsReady())
		return false;

	RenderShadows(conf, dev);
	BeginFrame(dev);
	RenderGeometry(conf, dev);
	RenderLights(conf, dev);

	if (Config::Current::EnableFlags::PhysicsDebugDraw) {
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		auto *sc = dynamic_cast<::Core::Scene::GameScene*>(conf.Scene);
		if (sc) {
			sc->GetPhysicsEngine().DoDebugDraw(dev);
		}

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}

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
 
bool DereferredPipeline::RenderShadows(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev) {
	auto *lconf = conf.Scene->GetLightConfig();
	if (!lconf) return true;
	  
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);  

	RenderSpotLightsShadows(conf, lconf->SpotLights, dev);

	//float Width = (float)::Settings->Window.Width;
	//float Height = (float)::Settings->Window.Height;
	//glViewport(0, 0, Width, Height);
	//dev.SetD3Mode();

	return true;
}

bool DereferredPipeline::RenderSpotLightsShadows(const MoonGlare::Core::MoveConfig &conf, Light::SpotLightList &lights, cRenderDevice& dev) {
	if (lights.empty()) return true;
	//glEnable(GL_CULL_FACE); 
	//glCullFace(GL_FRONT);
	for (auto &light : lights) {
		if (!light->CastShadows)
			continue;
		dev.SetCameraMatrix(light->LightMatrix); 
		light->ShadowMap.BindAndClear();
		dev.Bind(m_ShadowMapShader); 
		m_ShadowMapShader->SetLightPosition(light->Position);
		for (auto it : conf.RenderList) {
			dev.SetModelMatrix(it.first);
			it.second->DoRenderMesh(dev);
		}
	}           
	//glDisable(GL_CULL_FACE); 
	return true;     
}
 
bool DereferredPipeline::RenderGeometry(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev) {
	dev.Bind(m_GeometryShader);
	m_Buffer.BeginGeometryPass(); 
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);  

	//dev.Bind(conf.Camera);
	dev.SetModelMatrix(math::mat4());
	for (auto it : conf.RenderList) {
		dev.SetModelMatrix(it.first);
		it.second->DoRender(dev);
	}

	return true;
}

bool DereferredPipeline::RenderLights(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev) {
	auto *lconf = conf.Scene->GetLightConfig();
	if (!lconf) return true;
	glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
	RenderPointLights(lconf->PointLights, dev);
	RenderDirectionalLights(lconf->DirectionalLights, dev);
	RenderSpotLights(lconf->SpotLights, dev);
	return true;
}

bool DereferredPipeline::RenderPointLights(Light::PointLightList &lights, cRenderDevice& dev) {
	if (lights.empty()) return true;

	StencilTestEnabler Stencil;

	for (auto &light : lights) {
		math::mat4 mat; 
		mat = glm::translate(mat, light->Position);
		mat = glm::scale(mat, math::vec3(light->InfluenceRadius));
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

		dev.SetModelMatrix(mat);
		m_Sphere->DoRender(dev);
//light pass	  
		dev.Bind(m_PointLightShader); 
		m_Buffer.BeginLightingPass(); 
		m_PointLightShader->Bind(*light); 

		glEnable(GL_BLEND);    
		glBlendEquation(GL_FUNC_ADD); 
		glBlendFunc(GL_ONE, GL_ONE); 
		glDisable(GL_DEPTH_TEST); 
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 	 
		glEnable(GL_CULL_FACE);  
		glCullFace(GL_FRONT);  
		           
		dev.SetModelMatrix(mat); 
		m_Sphere->DoRender(dev);  

		glCullFace(GL_BACK);
		glDisable(GL_BLEND); 
		glDisable(GL_CULL_FACE);  
	}

	return true;
}

bool DereferredPipeline::RenderDirectionalLights(Light::DirectionalLightList &lights, cRenderDevice& dev) {
	if (lights.empty()) return true;

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
	for (auto &light : lights) {
		m_DirectionalLightShader->Bind(*light);
		m_DirectionalQuad.DrawElements(4, 0, 0, GL_QUADS);
	}
	m_DirectionalQuad.UnBind();
	glDisable(GL_BLEND);
	return true;
}
 
bool DereferredPipeline::RenderSpotLights(Light::SpotLightList &lights, cRenderDevice& dev) {
	if (lights.empty()) return true;

	StencilTestEnabler Stencil;

	for (auto &light : lights) {
		//math::mat4 mat = glm::lookAt(light->Position, light->Position - light->Direction, glm::vec3(0, -1, 0));
		math::mat4 mat = glm::lookAt(math::vec3(), -light->Direction, glm::vec3(0, -1, 0));//TODO: calculate up vector!
		mat = glm::translate(math::mat4(), light->Position) * mat;

		//auto scale = math::vec3(light->InfluenceDistance);
		//auto mat = light->ViewMatrix;
		auto scale = math::vec3(light->DistanceRadius);// , light->DistanceRadius, light->InfluenceDistance);
		for (int i = 0; i < 3; ++i) 
			mat[i] *= scale[i];  
//stencil pass
		m_Buffer.BeginStencilPass(); 
		dev.Bind(m_StencilShader); 
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE); 
		glClear(GL_STENCIL_BUFFER_BIT);
		 
		glStencilFunc(GL_ALWAYS, 0, 0);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP); 
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		dev.SetModelMatrix(mat);
		m_Cone->DoRender(dev); 
	
//light pass
		dev.Bind(m_SpotLightShader);
		m_Buffer.BeginLightingPass();   
		m_SpotLightShader->Bind(*light); 

		m_SpotLightShader->BindShadowMap(light->ShadowMap);
		m_SpotLightShader->SetLightMatrix(light->LightMatrix);

		glEnable(GL_BLEND);      
		glBlendEquation(GL_FUNC_ADD);    
		glBlendFunc(GL_ONE, GL_ONE);  
		glDisable(GL_DEPTH_TEST); 
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 	 
		glEnable(GL_CULL_FACE);    
		glCullFace(GL_FRONT);    

		glActiveTexture(GL_TEXTURE0);   
		glBindTexture(GL_TEXTURE_2D, 0);

		dev.SetModelMatrix(mat);   
		m_Cone->DoRender(dev);  

		//glActiveTexture(GL_TEXTURE0 + 5); 
		//GLint whichID;
		//glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);
		//glGetUniformiv(m_SpotLightShader->Handle(), m_SpotLightShader->Location("PlaneShadowMap"), &whichID);
		 
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
