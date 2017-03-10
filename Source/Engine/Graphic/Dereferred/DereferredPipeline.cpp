#include <pch.h>
#include <MoonGlare.h>

#include "DereferredPipeline.h"

#include <Renderer/RenderInput.h>

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Resources/ResourceManager.h>

namespace Graphic {
namespace Dereferred {

void DereferredPipeline::Initialize(World *world) {
    assert(world);
    m_World = world;

    try {
        if (!m_Buffer.Reset()) 
			throw "Unable to initialize render buffers!";

        auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();

        if (!shres.Load(m_ShaderShadowMapHandle, "ShadowMap")) throw 10;
        if (!shres.Load(m_ShaderLightSpotHandle, "Deferred/LightSpot")) throw 11;
        if (!shres.Load(m_ShaderLightPointHandle, "Deferred/LightPoint")) throw 12;
        if (!shres.Load(m_ShaderLightDirectionalHandle, "Deferred/LightDirectional")) throw 13;
        if (!shres.Load(m_ShaderStencilHandle, "Deferred/Stencil")) throw 15;
        if (!shres.Load(m_ShaderGeometryHandle, "Deferred/Geometry")) throw 16;
    }
    catch (int idx) {						 
        AddLogf(Error, "Unable to load shader with index %d", idx);
        throw "Unable to load shader";
    }
    catch (const char* msg) {
        AddLog(Error, msg);
        throw msg;
    }

    m_DefferedSink = std::make_unique<DefferedSink>();
    m_DefferedSink->Initialize(m_World->GetRendererFacade());
        
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
}     
       
void DereferredPipeline::Finalize() { 
    for (auto &it : m_PlaneShadowMapBuffer)
        it.Free();
    //if (m_Sphere) m_Sphere->Finalize();
}

//--------------------------------------------------------------------------------------

bool DereferredPipeline::InitializeDirectionalQuad() {
    NormalVector Normals{
        math::vec3(0.0f, 0.0f, 1.0f),
        math::vec3(0.0f, 0.0f, 1.0f),
        math::vec3(0.0f, 0.0f, 1.0f),
        math::vec3(0.0f, 0.0f, 1.0f),
    };
    VertexVector Vertex{
        math::vec3(-1.0f, -1.0f, 0.0f),
        math::vec3(1.0f, -1.0f, 0.0f),
        math::vec3(1.0f,  1.0f, 0.0f),
        math::vec3(-1.0f,  1.0f, 0.0f),
    };
    TexCoordVector Texture{
        math::vec2(1.0f, 1.0f),
        math::vec2(0.0f, 1.0f),
        math::vec2(0.0f, 0.0f),
        math::vec2(1.0f, 0.0f),
    };
    IndexVector Index{
        0, 1, 2, 3,
    };

    m_DirectionalQuad.Initialize(Vertex, Texture, Normals, Index);

    return true;
}

//--------------------------------------------------------------------------------------
 
bool DereferredPipeline::Execute(const MoonGlare::Core::MoveConfig &conf, cRenderDevice& dev) {
    auto ri = conf.m_RenderInput.get();
	auto *res = m_World->GetRendererFacade()->GetResourceManager();
	auto &shres = res->GetShaderResource();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    RenderSpotLightsShadows(conf, ri, dev);

    dev.ResetViewPort();
    m_Buffer.BeginFrame();

	{//geometry
		auto she = shres.GetExecutor<GeometryShaderDescriptor>(m_ShaderGeometryHandle);
		using Uniform = GeometryShaderDescriptor::Uniform;
		using Sampler = GeometryShaderDescriptor::Sampler;

		she.Bind();
		she.Set<Uniform::CameraMatrix>(ri->m_Camera.GetProjectionMatrix());

		m_Buffer.BeginGeometryPass();
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		auto &layers = conf.m_BufferFrame->GetCommandLayers();
		layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedGeometry>().Execute();
	}

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    RenderDirectionalLights(ri, dev);
    RenderSpotLights(ri, dev);
    RenderPointLights(ri, dev);

    auto size = dev.GetContext()->Size();
    m_Buffer.BeginFinalPass();
    glBlitFramebuffer(0, 0, size[0], size[1],
        0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);

    GetRenderDevice()->CheckError();

    return true;
}

//--------------------------------------------------------------------------------------
 
bool DereferredPipeline::RenderSpotLightsShadows(const MoonGlare::Core::MoveConfig &conf, RenderInput *ri, cRenderDevice& dev) {
    if (ri->m_SpotLights.empty()) return true;

    auto &shres = m_World->GetRendererFacade()->GetResourceManager()->GetShaderResource();
    auto she = shres.GetExecutor(m_ShaderShadowMapHandle);
    using Uniform = Shaders::ShadowMapShaderDescriptor::Uniform;

    she.Bind();
	auto &layers = conf.m_BufferFrame->GetCommandLayers();
	auto &ll = layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedLighting>();

    int index = 0;
    for (auto &light : ri->m_SpotLights) {
        if (!light.m_Base.m_Flags.m_CastShadows)
            continue;

        auto &sm = m_PlaneShadowMapBuffer[index];
        if (!sm)
            sm.New();
        ++index;

        sm.BindAndClear();

        she.Set<Uniform::CameraMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_ViewMatrix));
		ll.Execute();
    }           
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

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

void DefferedSink::Initialize(Renderer::RendererFacade *Renderer) {
    m_Renderer = Renderer;
    auto &shres = Renderer->GetResourceManager()->GetShaderResource();

    if (!shres.Load(m_ShaderGeometryHandle, "Deferred/Geometry")) throw "CANNOT LOAD D/G SHADER!";
    if (!shres.Load(m_ShaderShadowMapHandle, "ShadowMap")) throw "CANNOT LOAD ShadowMap SHADER!";
}

void DefferedSink::Reset(Renderer::Frame *frame) {
    auto &shres = m_Renderer->GetResourceManager()->GetShaderResource();
    m_frame = frame;

    auto &layers = m_frame->GetCommandLayers();
    m_GeometryQueue = &layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedGeometry>();
    m_LightQueue = &layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedLighting>();

    m_GeometryShader = shres.GetBuilder(*m_GeometryQueue, m_ShaderGeometryHandle);
    m_GeometryShader.Bind();

    m_ShadowShader = shres.GetBuilder(*m_LightQueue, m_ShaderShadowMapHandle);
}

DefferedSink::RObj DefferedSink::Begin(const math::mat4 & ModelMatrix, const Graphic::VAO & vao) {
    {
        using Uniform = GeometryShaderDescriptor::Uniform;
        m_GeometryShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(ModelMatrix));
        m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1));
        m_GeometryQueue->PushCommand<Renderer::Commands::VAOBind>()->m_VAO = vao.Handle();
    }

    {
        using Uniform = Shaders::ShadowMapShaderDescriptor::Uniform;
        m_ShadowShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(ModelMatrix));
        m_LightQueue->PushCommand<Renderer::Commands::VAOBind>()->m_VAO = vao.Handle();
    }

    return RObj {
        this,
    };
}

void DefferedSink::Mesh(Renderer::MaterialResourceHandle material, unsigned NumIndices, unsigned BaseIndex, unsigned BaseVertex, unsigned ElementsType) {
    using Uniform = GeometryShaderDescriptor::Uniform;
    using Sampler = GeometryShaderDescriptor::Sampler;

    if (material.m_TmpGuard == material.GuardValue) {
        auto matptr = m_Renderer->GetResourceManager()->GetMaterialManager().GetMaterial(material);
        if (matptr) {
            m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1));
            m_GeometryShader.Set<Sampler::DiffuseMap>(matptr->m_DiffuseMap);
        }
        else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1));
        }
    }
    else {
        //if (mat) 
        //	dev.Bind(mat->GetMaterial());
        //else
        //	dev.BindNullMaterial();
        assert(false);
        return;
    }

    auto garg = m_GeometryQueue->PushCommand<Renderer::Commands::VAODrawTrianglesBaseVertex>();
    garg->m_NumIndices = NumIndices;
    garg->m_IndexValueType = ElementsType;
    garg->m_BaseIndex = BaseIndex;
    garg->m_BaseVertex = BaseVertex;

	auto larg = m_LightQueue->PushCommand<Renderer::Commands::VAODrawTrianglesBaseVertex>();
	*larg = *garg;
}

} //namespace Dereferred
} //namespace Graphic 
