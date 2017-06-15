#include <pch.h>
#include <MoonGlare.h>

#include "DereferredPipeline.h"

#include <Renderer/RenderInput.h>

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Resources/ResourceManager.h>

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/FramebufferCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Device/Types.h>

namespace Graphic {
namespace Dereferred {

void DereferredPipeline::Initialize(World *world) {
    assert(world);
    m_World = world;

    m_ScreenSize = emath::MathCast<emath::fvec2>(math::fvec2(GetRenderDevice()->GetContextSize()));

    try {
        if (!m_Buffer.Reset()) 
            throw "Unable to initialize render buffers!";
    }
    catch (int idx) {						 
        AddLogf(Error, "Unable to load shader with index %d", idx);
        throw "Unable to load shader";
    }
    catch (const char* msg) {
        AddLog(Error, msg);
        throw msg;
    }

    m_DefferedSink = mem::make_aligned<DefferedSink>();
    m_DefferedSink->Initialize(m_World->GetRendererFacade());
    m_DefferedSink->m_DereferredPipeline = this;
        
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

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

void DefferedSink::Initialize(Renderer::RendererFacade *Renderer) {
    m_Renderer = Renderer;
    auto &shres = Renderer->GetResourceManager()->GetShaderResource();

    if (!shres.Load(m_ShaderGeometryHandle, "Deferred/Geometry")) throw "CANNOT LOAD D/G SHADER!";
    if (!shres.Load(m_ShaderShadowMapHandle, "ShadowMap")) throw "CANNOT LOAD ShadowMap SHADER!";
    if (!shres.Load(m_ShaderLightDirectionalHandle, "Deferred/LightDirectional")) throw "CANNOT LOAD D/DL SHADER!";
    if (!shres.Load(m_ShaderLightPointHandle, "Deferred/LightPoint")) throw "CANNOT LOAD D/PL SHADER!";
    if (!shres.Load(m_ShaderStencilHandle, "Deferred/Stencil")) throw "CANNOT LOAD D/PL SHADER!";
    if (!shres.Load(m_ShaderLightSpotHandle, "Deferred/LightSpot")) throw "CANNOT LOAD D/SL SHADER!";
}

void DefferedSink::Reset(Renderer::Frame *frame) {
    namespace Commands = Renderer::Commands;

    m_frame = frame;

    auto &shres = m_Renderer->GetResourceManager()->GetShaderResource();
    auto &layers = m_frame->GetCommandLayers();

//------------------------------------------------------------------------------------------
    m_LightGeometryQueue = frame->AllocateSubQueue();
    m_GeometryQueue = frame->AllocateSubQueue();
        //&layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedGeometry>();
    m_GeometryShader = shres.GetBuilder(*m_GeometryQueue, m_ShaderGeometryHandle);

    {
        using Uniform = GeometryShaderDescriptor::Uniform;
        using Sampler = GeometryShaderDescriptor::Sampler;

        //dev.ResetViewPort();
        auto &Size = m_DereferredPipeline->m_ScreenSize;
        m_GeometryQueue->MakeCommand<Commands::SetViewport>(0, 0, static_cast<int>(Size[0]), static_cast<int>(Size[1]));

        //m_Buffer.BeginFrame();
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
        m_GeometryQueue->MakeCommand<Commands::FramebufferDrawBind>(m_DereferredPipeline->m_Buffer.m_FrameBuffer);
        //glDrawBuffer(GL_COLOR_ATTACHMENT4);
        //glClear(GL_COLOR_BUFFER_BIT);
        m_GeometryQueue->MakeCommand<Commands::Clear>((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        m_GeometryShader.Bind();
        m_GeometryShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());

        //m_Buffer.BeginGeometryPass();
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
        static const GLenum DrawBuffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
        };
        //glDrawBuffers(4, DrawBuffers);
        m_GeometryQueue->MakeCommand<Commands::SetDrawBuffers>(4, DrawBuffers);

        m_GeometryQueue->MakeCommand<Commands::DepthMask>((GLboolean)GL_TRUE);
        m_GeometryQueue->MakeCommand<Commands::Clear>((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        m_GeometryQueue->MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
        m_GeometryQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    }
//------------------------------------------------------------------------------------------
    {
        using Uniform = DirectionalLightShaderDescriptor::Uniform;

        m_DirectionalLightQueue = frame->AllocateSubQueue();
        m_DirectionalLightShader = shres.GetBuilder(*m_DirectionalLightQueue, m_ShaderLightDirectionalHandle);
        m_DirectionalLightShader.Bind();
        m_DirectionalLightShader.Set<Uniform::ScreenSize>(m_DereferredPipeline->m_ScreenSize);
        //she.Set<Uniform::CameraPos>(ri->m_Camera.m_Position);
        //she.Set<Uniform::CameraMatrix>(emath::MathCast<emath::fmat4>(math::mat4()));
        //she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(math::mat4()));

        m_DirectionalLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
        m_DirectionalLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
        m_DirectionalLightQueue->MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_ONE, (GLenum)GL_ONE);
        m_DirectionalLightQueue->PushCommand<Commands::VAOBind>()->m_VAO = m_DereferredPipeline->m_DirectionalQuad.Handle();

        //m_Buffer.BeginLightingPass();
        m_DirectionalLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
        for (unsigned int i = 0; i < DereferredFrameBuffer::Buffers::MaxValue; i++) {
            m_DirectionalLightQueue->MakeCommand<Commands::Texture2DBindUnit>(m_DereferredPipeline->m_Buffer.m_Textures[i], i);
        }

    }
//------------------------------------------------------------------------------------------
    {
        m_StencilShader = shres.GetBuilder(*m_DirectionalLightQueue, m_ShaderStencilHandle);
    }
//------------------------------------------------------------------------------------------
    {
        using Uniform = PointLightShaderDescriptor::Uniform;

        m_PointLightQueue = frame->AllocateSubQueue();
        m_PointLightShader = shres.GetBuilder(*m_PointLightQueue, m_ShaderLightPointHandle);

        m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_STENCIL_TEST);
        m_PointLightShader.Bind();//TODO
        m_PointLightShader.Set<Uniform::ScreenSize>(m_DereferredPipeline->m_ScreenSize);
    }
//------------------------------------------------------------------------------------------
    {
        using Uniform = SpotLightShaderDescriptor::Uniform;

        m_SpotLightQueue = frame->AllocateSubQueue();
        m_SpotShader = shres.GetBuilder(*m_SpotLightQueue, m_ShaderLightSpotHandle);

        m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_STENCIL_TEST);
        m_SpotShader.Bind();//TODO
        m_SpotShader.Set<Uniform::ScreenSize>(m_DereferredPipeline->m_ScreenSize);
    }
//------------------------------------------------------------------------------------------
    {
        m_SpotLightShadowQueue = &layers.Get<Renderer::Configuration::FrameBuffer::Layer::ShadowMaps>();
        m_ShadowShader = shres.GetBuilder(*m_SpotLightShadowQueue, m_ShaderShadowMapHandle);
        m_ShadowShader.Bind();

        m_SpotLightShadowQueue->MakeCommand<Commands::DepthMask>((GLboolean)GL_TRUE);
        m_SpotLightShadowQueue->MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
        m_SpotLightShadowQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    }
    {
        auto &q = layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedGeometry>();

        q.PushQueue(m_GeometryQueue);
        q.MakeCommand<Commands::DepthMask>((GLboolean)GL_FALSE);
        q.MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
        q.PushQueue(m_DirectionalLightQueue);
        q.MakeCommand<Commands::Enable>((GLenum)GL_STENCIL_TEST);
        q.PushQueue(m_SpotLightQueue);
        q.PushQueue(m_PointLightQueue);
        q.MakeCommand<Commands::Disable>((GLenum)GL_STENCIL_TEST);

        //m_Buffer.BeginFinalPass();
        q.MakeCommand<Commands::FramebufferDrawBind>(Renderer::Device::InvalidFramebufferHandle);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffer);
        q.MakeCommand<Commands::FramebufferReadBind>(m_DereferredPipeline->m_Buffer.m_FrameBuffer);
        q.MakeCommand<Commands::SetReadBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
        
        auto size = m_DereferredPipeline->m_ScreenSize;
        q.MakeCommand<Commands::BlitFramebuffer>(
            0, 0, (GLint)size[0], (GLint)size[1], 
            0, 0, (GLint)size[0], (GLint)size[1], 
            (GLenum)GL_COLOR_BUFFER_BIT, (GLenum)GL_LINEAR);
        //glBlitFramebuffer(0, 0, size[0], size[1], 0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
}

DefferedSink::RObj DefferedSink::Begin(const math::mat4 & ModelMatrix, const Graphic::VAO & vao) {
    namespace Commands = Renderer::Commands;
    {
        using Uniform = GeometryShaderDescriptor::Uniform;
        m_GeometryShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(ModelMatrix));
        m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1));

        m_GeometryQueue->PushCommand<Commands::VAOBind>()->m_VAO = vao.Handle();
    }

    {
        using Uniform = Shaders::ShadowMapShaderDescriptor::Uniform;
        m_ShadowShader.m_Queue = m_LightGeometryQueue;
        m_ShadowShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(ModelMatrix));
        m_LightGeometryQueue->PushCommand<Commands::VAOBind>()->m_VAO = vao.Handle();
    }

    return RObj {
        this,
    };
}

void DefferedSink::Mesh(Renderer::MaterialResourceHandle material, unsigned NumIndices, unsigned BaseIndex, unsigned BaseVertex, unsigned ElementsType) {
    namespace Commands = Renderer::Commands;
    using Uniform = GeometryShaderDescriptor::Uniform;
    using Sampler = GeometryShaderDescriptor::Sampler;

    if (material.m_TmpGuard == material.GuardValue) {
        auto matptr = m_Renderer->GetResourceManager()->GetMaterialManager().GetMaterial(material);
        if (matptr) {
            m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1));
            m_GeometryShader.Set<Sampler::DiffuseMap>(matptr->m_DiffuseMap);
        }
        else {
            m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1));
            m_GeometryShader.Set<Sampler::DiffuseMap>(Renderer::Device::InvalidTextureHandle);
        }
    } else {
        //if (mat) 
        //	dev.Bind(mat->GetMaterial());
        //else
        //	dev.BindNullMaterial();
        assert(false);
        return;
    }

    auto garg = m_GeometryQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
    garg->m_NumIndices = NumIndices;
    garg->m_IndexValueType = ElementsType;
    garg->m_BaseIndex = BaseIndex;
    garg->m_BaseVertex = BaseVertex;

    auto larg = m_LightGeometryQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
    *larg = *garg;
}

void DefferedSink::SubmitDirectionalLight(const Renderer::Light::LightBase & linfo) {
    namespace Commands = Renderer::Commands;
    using Uniform = DirectionalLightShaderDescriptor::Uniform;

    m_DirectionalLightShader.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Color));
    m_DirectionalLightShader.Set<Uniform::AmbientIntensity>(linfo.m_AmbientIntensity);
    m_DirectionalLightShader.Set<Uniform::DiffuseIntensity>(linfo.m_DiffuseIntensity);
    //she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);

    auto garg = m_DirectionalLightQueue->PushCommand<Commands::VAODrawElements>();
    garg->m_NumIndices     = 4;
    garg->m_IndexValueType = GL_UNSIGNED_INT;// m_DereferredPipeline->m_DirectionalQuad.IndexValueType();
    garg->m_ElementMode    = GL_QUADS;
}

void DefferedSink::SubmitPointLight(const Renderer::Light::PointLight & linfo) {
    namespace Commands = Renderer::Commands;
    //glDrawBuffer(GL_NONE);  //m_Buffer.BeginStencilPass();
    m_PointLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_NONE);

    {
        using Uniform = StencilLightShaderDescriptor::Uniform;
        m_StencilShader.m_Queue = m_PointLightQueue;
        m_StencilShader.Bind();
        m_StencilShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());
        m_StencilShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_PositionMatrix));
    }

    m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
    m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
    m_PointLightQueue->MakeCommand<Commands::Clear>((GLbitfield)GL_STENCIL_BUFFER_BIT);

    m_PointLightQueue->MakeCommand<Commands::StencilFunc>((GLenum)GL_ALWAYS, 0, 0u);
    m_PointLightQueue->MakeCommand<Commands::StencilOpSeparate>((GLenum)GL_BACK, (GLenum)GL_KEEP, (GLenum)GL_INCR_WRAP, (GLenum)GL_KEEP);
    m_PointLightQueue->MakeCommand<Commands::StencilOpSeparate>((GLenum)GL_FRONT, (GLenum)GL_KEEP, (GLenum)GL_DECR_WRAP, (GLenum)GL_KEEP);

    using Uniform = PointLightShaderDescriptor::Uniform;

    auto &vao = m_DereferredPipeline->m_Sphere->GetVAO();
    m_PointLightQueue->PushCommand<Commands::VAOBind>()->m_VAO = vao.Handle();

    auto garg = m_PointLightQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
    auto &mesh = m_DereferredPipeline->m_Sphere->GetMeshVector();
    garg->m_NumIndices     = mesh[0].NumIndices;
    garg->m_IndexValueType = vao.IndexValueType();
    garg->m_BaseIndex      = mesh[0].BaseIndex;
    garg->m_BaseVertex     = mesh[0].BaseVertex;

    {
        using Uniform = PointLightShaderDescriptor::Uniform;
        m_PointLightShader.Bind();
        m_PointLightShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());
        m_PointLightShader.Set<Uniform::CameraPos>(m_Camera.m_Position);

        //m_Buffer.BeginLightingPass();
        m_PointLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
        for (unsigned int i = 0; i < DereferredFrameBuffer::Buffers::MaxValue; i++) {
            m_PointLightQueue->MakeCommand<Commands::Texture2DBindUnit>(m_DereferredPipeline->m_Buffer.m_Textures[i], i);
        }

        //	she.Set<Uniform::LightMatrix>(emath::MathCast<emath::fmat4>((math::mat4)light.m_ViewMatrix));

        m_PointLightShader.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Base.m_Color));
        m_PointLightShader.Set<Uniform::AmbientIntensity>(linfo.m_Base.m_AmbientIntensity);
        m_PointLightShader.Set<Uniform::DiffuseIntensity>(linfo.m_Base.m_DiffuseIntensity);

        //	she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);

        m_PointLightShader.Set<Uniform::Position>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Position));
        m_PointLightShader.Set<Uniform::AttenuationLinear>(linfo.m_Attenuation.m_Linear);
        m_PointLightShader.Set<Uniform::AttenuationExp>(linfo.m_Attenuation.m_Exp);
        m_PointLightShader.Set<Uniform::AttenuationConstant>(linfo.m_Attenuation.m_Constant);
        m_PointLightShader.Set<Uniform::AttenuationMinThreshold>(linfo.m_Attenuation.m_Threshold);
        m_PointLightShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_PositionMatrix));
    }

    m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
    m_PointLightQueue->MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_ONE, (GLenum)GL_ONE);
    m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
    m_PointLightQueue->MakeCommand<Commands::StencilFunc>((GLenum)GL_NOTEQUAL, 0, 0xFFu);
    m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_CULL_FACE);
    m_PointLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_FRONT);

    m_PointLightQueue->MakeCommand<Commands::Texture2DBindUnit>(0u, 0u);
    m_PointLightQueue->PushCommand<Commands::VAOBind>()->m_VAO = vao.Handle();
    m_PointLightQueue->MakeCommand<Commands::VAODrawTrianglesBaseVertex>(*garg);

    m_PointLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_BACK);
    m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
}

void DefferedSink::SubmitSpotLight(const Renderer::Light::SpotLight &linfo) {
    namespace Commands = Renderer::Commands;

    Renderer::PlaneShadowMap *sm = nullptr;
    if (linfo.m_Base.m_Flags.m_CastShadows) {
        sm = m_frame->AllocatePlaneShadowMap();
        if (sm) {

            using Uniform = Shaders::ShadowMapShaderDescriptor::Uniform;

            //sm->BindAndClear();
            m_SpotLightShadowQueue->MakeCommand<Commands::SetViewport>(0, 0, static_cast<int>(sm->size), static_cast<int>(sm->size));
            m_SpotLightShadowQueue->MakeCommand<Commands::FramebufferDrawBind>(sm->framebufferHandle);
            m_SpotLightShadowQueue->MakeCommand<Commands::Clear>((GLbitfield)(GL_DEPTH_BUFFER_BIT));

            m_ShadowShader.m_Queue = m_SpotLightShadowQueue;
            m_ShadowShader.Set<Uniform::CameraMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_ViewMatrix));
            m_ShadowShader.m_Queue->PushQueue(m_LightGeometryQueue);
        }
    }

    //glDrawBuffer(GL_NONE);  //m_Buffer.BeginStencilPass();
    m_SpotLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_NONE);

    {
        using Uniform = StencilLightShaderDescriptor::Uniform;
        m_StencilShader.m_Queue = m_SpotLightQueue;
        m_StencilShader.Bind();
        m_StencilShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());
        m_StencilShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_PositionMatrix));
    }

    m_SpotLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
    m_SpotLightQueue->MakeCommand<Commands::Clear>((GLbitfield)GL_STENCIL_BUFFER_BIT);

    m_SpotLightQueue->MakeCommand<Commands::StencilFunc>((GLenum)GL_ALWAYS, 0, 0u);
    m_SpotLightQueue->MakeCommand<Commands::StencilOpSeparate>((GLenum)GL_BACK, (GLenum)GL_KEEP, (GLenum)GL_INCR_WRAP, (GLenum)GL_KEEP);
    m_SpotLightQueue->MakeCommand<Commands::StencilOpSeparate>((GLenum)GL_FRONT, (GLenum)GL_KEEP, (GLenum)GL_DECR_WRAP, (GLenum)GL_KEEP);

    auto &vao = m_DereferredPipeline->m_Sphere->GetVAO();
    auto &mesh = m_DereferredPipeline->m_Sphere->GetMeshVector();

    m_SpotLightQueue->PushCommand<Commands::VAOBind>()->m_VAO = vao.Handle();
    auto garg = m_SpotLightQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
    garg->m_NumIndices = mesh[0].NumIndices;
    garg->m_IndexValueType = vao.IndexValueType();
    garg->m_BaseIndex = mesh[0].BaseIndex;
    garg->m_BaseVertex = mesh[0].BaseVertex;

    //m_Buffer.BeginLightingPass();
    m_SpotLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
    for (unsigned int i = 0; i < DereferredFrameBuffer::Buffers::MaxValue; i++) {
        m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(m_DereferredPipeline->m_Buffer.m_Textures[i], i);
    }

    //sm->BindAsTexture(SamplerIndex::PlaneShadow);
    if(sm) {
        m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(sm->textureHandle, (unsigned)SamplerIndex::PlaneShadow);
    } else {
        m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(Renderer::Device::InvalidTextureHandle, (unsigned)SamplerIndex::PlaneShadow);
    }

    {
        using Uniform = SpotLightShaderDescriptor::Uniform;
        m_SpotShader.Bind();
        m_SpotShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());
        m_SpotShader.Set<Uniform::CameraPos>(m_Camera.m_Position);

        if(sm)
            m_SpotShader.Set<Uniform::ShadowMapSize>(emath::fvec2(sm->size, sm->size));

        m_SpotShader.Set<Uniform::LightMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_ViewMatrix));

        m_SpotShader.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Base.m_Color));
        m_SpotShader.Set<Uniform::AmbientIntensity>(linfo.m_Base.m_AmbientIntensity);
        m_SpotShader.Set<Uniform::DiffuseIntensity>(linfo.m_Base.m_DiffuseIntensity);

        m_SpotShader.Set<Uniform::EnableShadows>(linfo.m_Base.m_Flags.m_CastShadows ? 1 : 0);

        m_SpotShader.Set<Uniform::Position>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Position));
        m_SpotShader.Set<Uniform::Direction>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Direction));

        m_SpotShader.Set<Uniform::CutOff>(linfo.m_CutOff);

        m_SpotShader.Set<Uniform::AttenuationLinear>(linfo.m_Attenuation.m_Linear);
        m_SpotShader.Set<Uniform::AttenuationExp>(linfo.m_Attenuation.m_Exp);
        m_SpotShader.Set<Uniform::AttenuationConstant>(linfo.m_Attenuation.m_Constant);
        m_SpotShader.Set<Uniform::AttenuationMinThreshold>(linfo.m_Attenuation.m_Threshold);
        m_SpotShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_PositionMatrix));
    }

    m_SpotLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
    m_SpotLightQueue->MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_ONE, (GLenum)GL_ONE);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
    m_SpotLightQueue->MakeCommand<Commands::StencilFunc>((GLenum)GL_NOTEQUAL, 0, 0xFFu);
    m_SpotLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_CULL_FACE);
    m_SpotLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_FRONT);

    m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(0u, 0u);
    m_SpotLightQueue->PushCommand<Commands::VAOBind>()->m_VAO = vao.Handle();
    m_SpotLightQueue->MakeCommand<Commands::VAODrawTrianglesBaseVertex>(*garg);

    m_SpotLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_BACK);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
}

} //namespace Dereferred
} //namespace Graphic 
