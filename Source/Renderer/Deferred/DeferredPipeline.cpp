#define NEED_VAO_BUILDER
#define NEED_MESH_BUILDER


#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Resources/Mesh/MeshUpdate.h>

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/FramebufferCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Device/Types.h>

#include "DeferredPipeline.h"

namespace MoonGlare::Renderer::Deferred {

DeferredSink::DeferredSink() {
    SetStaticFog(StaticFog{});

    visibility = 1.0e10;
}

void DeferredSink::InitializeDirectionalQuad() {
    auto &mm = m_Renderer->GetResourceManager()->GetMeshManager();
    //auto &matm = m_Renderer->GetResourceManager()->GetMaterialManager();
    mm.Allocate(quadMesh);

    Resources::MeshData meshData;
    meshData.verticles = {
        math::vec3(-1.0f, -1.0f, 0.0f),
        math::vec3(1.0f, -1.0f, 0.0f),
        math::vec3(1.0f,  1.0f, 0.0f),
        math::vec3(-1.0f,  1.0f, 0.0f),
    };
    meshData.UV0 = {
        math::vec2(1.0f, 1.0f),
        math::vec2(0.0f, 1.0f),
        math::vec2(0.0f, 0.0f),
        math::vec2(1.0f, 0.0f),
    };
    meshData.index = {
        0, 1, 2, 0, 2, 3,
    };

    mm.SetMeshData(quadMesh, std::move(meshData));

    auto task = std::make_shared<Resources::Loader::CustomMeshLoader>(quadMesh, mm);
    task->materialArray = {};
    task->meshArray = {};
    auto &mesh = task->meshArray;
    mesh.valid = true;
    mesh.indexElementType = GL_UNSIGNED_INT;
    mesh.numIndices = 6;
    mesh.baseIndex = 0;
    mesh.baseVertex = 0;
    mesh.elementMode = GL_TRIANGLES;

    m_Renderer->GetAsyncLoader()->QueueTask(std::move(task));
}

//------------------------------------------------------------------------------------------

void DeferredSink::Initialize(RendererFacade *renderer) {
    m_Renderer = renderer;
    m_ScreenSize = renderer->GetContext()->GetSizef();
    auto &shres = renderer->GetResourceManager()->GetShaderResource();

    try {
        if (!m_Buffer.Reset(m_ScreenSize))
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

    if (!shres.Load(m_ShaderGeometryHandle, "Deferred/Geometry")) throw "CANNOT LOAD D/G SHADER!";
    if (!shres.Load(m_ShaderShadowMapHandle, "ShadowMap")) throw "CANNOT LOAD ShadowMap SHADER!";
    if (!shres.Load(m_ShaderLightDirectionalHandle, "Deferred/LightDirectional")) throw "CANNOT LOAD D/DL SHADER!";
    if (!shres.Load(m_ShaderLightPointHandle, "Deferred/LightPoint")) throw "CANNOT LOAD D/PL SHADER!";
    if (!shres.Load(m_ShaderStencilHandle, "Deferred/Stencil")) throw "CANNOT LOAD D/ST SHADER!";
    if (!shres.Load(m_ShaderLightSpotHandle, "Deferred/LightSpot")) throw "CANNOT LOAD D/SL SHADER!";

    auto &mm = m_Renderer->GetResourceManager()->GetMeshManager();

    if (!mm.LoadMesh("file:///Models/PointLightSphere.3ds", sphereMesh)) {     //TODO: remove direct uri
        AddLog(Error, "Cannot load sphere mesh!");
        throw std::runtime_error("Cannot load sphere mesh!");
    }
    if (!mm.LoadMesh("file:///Models/PointLightSphere.3ds", coneMesh)) {     //TODO: remove direct uri      SpotLightCone.3ds
        AddLog(Error, "Cannot load cone mesh!");
        throw std::runtime_error("Cannot load cone mesh!");
    }
    InitializeDirectionalQuad();
}

void DeferredSink::SetStaticFog(const StaticFog &afog) {
    fog = afog;
    fogSet = false;
    visibility = fog.m_Enabled ? fog.m_End + 1 : -1;
}

void DeferredSink::Reset(Frame *frame) {

    m_frame = frame;

    meshcouter = 0;

    auto &shres = m_Renderer->GetResourceManager()->GetShaderResource();
    auto &layers = m_frame->GetCommandLayers();

    auto SetFog = [this](auto builder) {
        builder.Set("gStaticFog.Enabled", fog.m_Enabled ? 1 : 0);
        if (fog.m_Enabled) {
            builder.Set("gStaticFog.Color", emath::MathCast<emath::fvec3>(fog.m_Color));
            builder.Set("gStaticFog.Start", fog.m_Start);
            builder.Set("gStaticFog.End", fog.m_End);
        }
    };

//------------------------------------------------------------------------------------------
    m_LightGeometryQueue = frame->AllocateSubQueue();
    m_GeometryQueue = frame->AllocateSubQueue();
        //&layers.Get<Configuration::FrameBuffer::Layer::DefferedGeometry>();
    m_GeometryShader = shres.GetBuilder(*m_GeometryQueue, m_ShaderGeometryHandle);

    {
        using Uniform = GeometryShaderDescriptor::Uniform;
        using Sampler = GeometryShaderDescriptor::Sampler;

        //dev.ResetViewPort();
        auto &Size = m_ScreenSize;
        m_GeometryQueue->MakeCommand<Commands::SetViewport>(0, 0, static_cast<int>(Size[0]), static_cast<int>(Size[1]));

        //m_Buffer.BeginFrame();
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
        m_GeometryQueue->MakeCommand<Commands::FramebufferDrawBind>(m_Buffer.m_FrameBuffer);
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
        if (!fogSet) {
            SetFog(m_DirectionalLightShader);
        }
        m_DirectionalLightShader.Set<Uniform::ScreenSize>(m_ScreenSize);
        m_DirectionalLightShader.Set<Uniform::CameraPos>(m_Camera.m_Position);
        //she.Set<Uniform::CameraMatrix>(emath::MathCast<emath::fmat4>(math::mat4()));
        //she.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(math::mat4()));

        m_DirectionalLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
        m_DirectionalLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
        m_DirectionalLightQueue->MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_ONE, (GLenum)GL_ONE);
        m_DirectionalLightQueue->MakeCommand<Commands::VAOBindResource>(quadMesh.deviceHandle);// ->m_VAO = m_DeferredPipeline->m_DirectionalQuad.Handle();

        //m_Buffer.BeginLightingPass();
        m_DirectionalLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
        for (unsigned int i = 0; i < DeferredFrameBuffer::Buffers::MaxValue; i++) {
            m_DirectionalLightQueue->MakeCommand<Commands::Texture2DBindUnit>(m_Buffer.m_Textures[i], i);
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
        if (!fogSet) {
            SetFog(m_PointLightShader);
        }
        m_PointLightShader.Set<Uniform::ScreenSize>(m_ScreenSize);
    }
//------------------------------------------------------------------------------------------
    {
        using Uniform = SpotLightShaderDescriptor::Uniform;

        m_SpotLightQueue = frame->AllocateSubQueue();
        m_SpotShader = shres.GetBuilder(*m_SpotLightQueue, m_ShaderLightSpotHandle);

        m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_STENCIL_TEST);
        m_SpotShader.Bind();//TODO
        if (!fogSet) {
            SetFog(m_SpotShader);
        }
        m_SpotShader.Set<Uniform::ScreenSize>(m_ScreenSize);
    }
//------------------------------------------------------------------------------------------
    {
        m_SpotLightShadowQueue = &layers.Get<Configuration::FrameBuffer::Layer::ShadowMaps>();
        m_ShadowShader = shres.GetBuilder(*m_SpotLightShadowQueue, m_ShaderShadowMapHandle);
        m_ShadowShader.Bind();

        m_SpotLightShadowQueue->MakeCommand<Commands::DepthMask>((GLboolean)GL_TRUE);
        m_SpotLightShadowQueue->MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
        m_SpotLightShadowQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    }
    {
        auto &qgeom = layers.Get<Configuration::FrameBuffer::Layer::DefferedGeometry>();
        qgeom.PushQueue(m_GeometryQueue);

        auto &qlight = layers.Get<Configuration::FrameBuffer::Layer::DefferedLighting>();
        qlight.MakeCommand<Commands::DepthMask>((GLboolean)GL_FALSE);
        qlight.MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
        qlight.PushQueue(m_DirectionalLightQueue);
        qlight.MakeCommand<Commands::Enable>((GLenum)GL_STENCIL_TEST);
        qlight.PushQueue(m_SpotLightQueue);
        qlight.PushQueue(m_PointLightQueue);
        qlight.MakeCommand<Commands::Disable>((GLenum)GL_STENCIL_TEST);


        auto &qpost = layers.Get<Configuration::FrameBuffer::Layer::PostRender>();
        //m_Buffer.BeginFinalPass();
        qpost.MakeCommand<Commands::FramebufferDrawBind>(Device::InvalidFramebufferHandle);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffer);
        qpost.MakeCommand<Commands::FramebufferReadBind>(m_Buffer.m_FrameBuffer);
        qpost.MakeCommand<Commands::SetReadBuffer>((GLenum)GL_COLOR_ATTACHMENT4);

        auto size = m_ScreenSize;
        qpost.MakeCommand<Commands::BlitFramebuffer>(
            0, 0, (GLint)size[0], (GLint)size[1],
            0, 0, (GLint)size[0], (GLint)size[1],
            (GLenum)GL_COLOR_BUFFER_BIT, (GLenum)GL_LINEAR);
        //glBlitFramebuffer(0, 0, size[0], size[1], 0, 0, size[0], size[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
}

void DeferredSink::Mesh(const emath::fmat4 &ModelMatrix, const emath::fvec3 &basepos, MeshResourceHandle meshH, MaterialResourceHandle matH) {
    //TODO

    auto &mm = m_Renderer->GetResourceManager()->GetMeshManager();
    auto *md = mm.GetMeshData(meshH);

    if (!md || !MeshVisibilityTest(basepos, md->boundingRadius))
        return;

    auto *meshes = mm.GetMeshes(meshH);

    if (!meshes)
        return;

    {
        using Uniform = PlaneShadowMapShaderDescriptor::Uniform;
        m_ShadowShader.m_Queue = m_LightGeometryQueue;
        m_ShadowShader.Set<Uniform::ModelMatrix>(ModelMatrix);
        m_LightGeometryQueue->PushCommand<Commands::VAOBind>()->m_VAO = *meshH.deviceHandle;// vao.Handle();
    }

    {
        using Sampler = GeometryShaderDescriptor::Sampler;
        using Uniform = GeometryShaderDescriptor::Uniform;
        m_GeometryShader.Set<Uniform::ModelMatrix>(ModelMatrix);

        m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1,1,1));
        m_GeometryShader.Set<Sampler::DiffuseMap>(Device::InvalidTextureHandle);

        m_GeometryQueue->PushCommand<Commands::VAOBind>()->m_VAO = *meshH.deviceHandle;// vao.Handle();
    }

    {
        auto &mesh = *meshes;
        //auto &mat = *materials;

        if (mesh.valid)
        {
            ++meshcouter;

            using Sampler = GeometryShaderDescriptor::Sampler;
            using Uniform = GeometryShaderDescriptor::Uniform;

            if (matH.deviceHandle) {
                m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1, 1, 1));
                m_GeometryShader.Set<Sampler::DiffuseMap>(matH.deviceHandle->mapTexture[0]);
            }
            else {
                m_GeometryShader.Set<Uniform::DiffuseColor>(emath::fvec3(1, 1, 1));
                m_GeometryShader.Set<Sampler::DiffuseMap>(Device::InvalidTextureHandle);
            }

            auto garg = m_GeometryQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
            garg->m_NumIndices = mesh.numIndices;
            garg->m_IndexValueType = mesh.indexElementType;
            garg->m_BaseIndex = mesh.baseIndex;
            garg->m_BaseVertex = mesh.baseVertex;

            auto larg = m_LightGeometryQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
            *larg = *garg;
        }
    }
}

void DeferredSink::SubmitDirectionalLight(const LightBase & linfo) {
    using Uniform = DirectionalLightShaderDescriptor::Uniform;

    m_DirectionalLightShader.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Color));
    m_DirectionalLightShader.Set<Uniform::AmbientIntensity>(linfo.m_AmbientIntensity);
    m_DirectionalLightShader.Set<Uniform::DiffuseIntensity>(linfo.m_DiffuseIntensity);
    //she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);

    auto garg = m_DirectionalLightQueue->PushCommand<Commands::VAODrawElements>();
    garg->m_NumIndices = 6;
    garg->m_IndexValueType = GL_UNSIGNED_INT;// m_DeferredPipeline->m_DirectionalQuad.IndexValueType();
    garg->m_ElementMode = GL_TRIANGLES;
}

bool DeferredSink::MeshVisibilityTest(const emath::fvec3 &position, float radius) {
    emath::fvec3 delta = m_Camera.m_Position - position;

    auto sqnorm = delta.squaredNorm();
    //if (sqnorm < pow(radius, 2.0f))
        //return true;

    if (sqnorm > pow(radius + visibility, 2.0f))
        return false;

    //if (delta.dot(m_Camera.m_Direction) < 0)
        //return false;

    return true;
}

bool DeferredSink::PointLightVisibilityTest(const emath::fvec3 &position, float radius) {
    emath::fvec3 delta = m_Camera.m_Position - position;

    auto sqnorm = delta.squaredNorm();
    if (sqnorm < pow(radius, 2.0f))
        return true;

    if (sqnorm > pow(radius + visibility, 2.0f))
        return false;

    if (delta.dot(m_Camera.m_Direction) < 0)
        return false;

    return true;
}

void DeferredSink::SubmitPointLight(const PointLight & linfo) {
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

    auto &mm = m_Renderer->GetResourceManager()->GetMeshManager();
    auto *mesh = mm.GetMeshes(sphereMesh);

    m_PointLightQueue->MakeCommand<Commands::VAOBindResource>(sphereMesh.deviceHandle);

    auto garg = m_PointLightQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
    garg->m_NumIndices = (*mesh).numIndices;
    garg->m_IndexValueType = (*mesh).indexElementType;
    garg->m_BaseIndex = (*mesh).baseIndex;
    garg->m_BaseVertex = (*mesh).baseVertex;

    {
        using Uniform = PointLightShaderDescriptor::Uniform;
        m_PointLightShader.Bind();
        m_PointLightShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());
        m_PointLightShader.Set<Uniform::CameraPos>(m_Camera.m_Position);

        //m_Buffer.BeginLightingPass();
        m_PointLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
        for (unsigned int i = 0; i < DeferredFrameBuffer::Buffers::MaxValue; i++) {
            m_PointLightQueue->MakeCommand<Commands::Texture2DBindUnit>(m_Buffer.m_Textures[i], i);
        }

        m_PointLightShader.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Base.m_Color));
        m_PointLightShader.Set<Uniform::AmbientIntensity>(linfo.m_Base.m_AmbientIntensity);
        m_PointLightShader.Set<Uniform::DiffuseIntensity>(linfo.m_Base.m_DiffuseIntensity);

        //	she.Set<Uniform::EnableShadows>(light.m_Base.m_Flags.m_CastShadows ? 1 : 0);

        m_PointLightShader.Set<Uniform::Position>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Position));
        m_PointLightShader.Set<Uniform::Attenuation>(emath::MathCast<emath::fvec4>((math::vec4)linfo.m_Attenuation.values));
        m_PointLightShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_PositionMatrix));
    }

     m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
     m_PointLightQueue->MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_ONE, (GLenum)GL_ONE);
     m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
     m_PointLightQueue->MakeCommand<Commands::StencilFunc>((GLenum)GL_NOTEQUAL, 0, 0xFFu);
     m_PointLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_CULL_FACE);
     m_PointLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_FRONT);

    m_PointLightQueue->MakeCommand<Commands::Texture2DBindUnit>(0u, 0u);
    m_PointLightQueue->MakeCommand<Commands::VAOBindResource>(sphereMesh.deviceHandle);
    m_PointLightQueue->MakeCommand<Commands::VAODrawTrianglesBaseVertex>(*garg);

    m_PointLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_BACK);
    m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    m_PointLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
}

void DeferredSink::SubmitSpotLight(const SpotLight &linfo) {
  // emath::fvec3 delta = m_Camera.m_Position - emath::MathCast<emath::fvec3>((math::fvec3)linfo.m_Position);
 //  if (delta.squaredNorm() > 100.0f)
 //      return;

    PlaneShadowMap *sm = nullptr;
    if (linfo.m_Base.m_Flags.m_CastShadows) {
        sm = m_frame->AllocatePlaneShadowMap();
        if (sm) {

            using Uniform = PlaneShadowMapShaderDescriptor::Uniform;

            //sm->BindAndClear();
            m_SpotLightShadowQueue->MakeCommand<Commands::FramebufferDrawBind>(sm->framebufferHandle);
            m_SpotLightShadowQueue->MakeCommand<Commands::SetViewport>(0, 0, static_cast<int>(sm->size), static_cast<int>(sm->size));
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


    auto &mm = m_Renderer->GetResourceManager()->GetMeshManager();
    auto *mesh = mm.GetMeshes(coneMesh);


    m_SpotLightQueue->MakeCommand<Commands::VAOBindResource>(coneMesh.deviceHandle);
    auto garg = m_SpotLightQueue->PushCommand<Commands::VAODrawTrianglesBaseVertex>();
    garg->m_NumIndices = (*mesh).numIndices;
    garg->m_IndexValueType = (*mesh).indexElementType;
    garg->m_BaseIndex = (*mesh).baseIndex;
    garg->m_BaseVertex = (*mesh).baseVertex;

    //m_Buffer.BeginLightingPass();
    m_SpotLightQueue->MakeCommand<Commands::SetDrawBuffer>((GLenum)GL_COLOR_ATTACHMENT4);
    for (unsigned int i = 0; i < DeferredFrameBuffer::Buffers::MaxValue; i++) {
        m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(m_Buffer.m_Textures[i], i);
    }

    //sm->BindAsTexture(SamplerIndex::PlaneShadow);
    if (sm) {
        m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(sm->textureHandle, (unsigned)SamplerIndex::PlaneShadow);
    }
    else {
        m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(Device::InvalidTextureHandle, (unsigned)SamplerIndex::PlaneShadow);
    }

    {
        using Uniform = SpotLightShaderDescriptor::Uniform;
        m_SpotShader.Bind();
        m_SpotShader.Set<Uniform::CameraMatrix>(m_Camera.GetProjectionMatrix());
        m_SpotShader.Set<Uniform::CameraPos>(m_Camera.m_Position);

        if (sm)
            m_SpotShader.Set<Uniform::ShadowMapSize>(emath::fvec2(sm->size, sm->size));

        m_SpotShader.Set<Uniform::LightMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_ViewMatrix));

        m_SpotShader.Set<Uniform::Color>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Base.m_Color));
        m_SpotShader.Set<Uniform::AmbientIntensity>(linfo.m_Base.m_AmbientIntensity);
        m_SpotShader.Set<Uniform::DiffuseIntensity>(linfo.m_Base.m_DiffuseIntensity);

        m_SpotShader.Set<Uniform::EnableShadows>(linfo.m_Base.m_Flags.m_CastShadows ? 1 : 0);

        m_SpotShader.Set<Uniform::Position>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Position));
        m_SpotShader.Set<Uniform::Direction>(emath::MathCast<emath::fvec3>((math::vec3)linfo.m_Direction));

        m_SpotShader.Set<Uniform::CutOff>(linfo.m_CutOff);
        m_SpotShader.Set<Uniform::Attenuation>(emath::MathCast<emath::fvec4>((math::vec4)linfo.m_Attenuation.values));

        m_SpotShader.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>((math::mat4)linfo.m_PositionMatrix));
    }

    m_SpotLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_BLEND);
    m_SpotLightQueue->MakeCommand<Commands::Blend>((GLenum)GL_FUNC_ADD, (GLenum)GL_ONE, (GLenum)GL_ONE);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
    m_SpotLightQueue->MakeCommand<Commands::StencilFunc>((GLenum)GL_NOTEQUAL, 0, 0xFFu);
    m_SpotLightQueue->MakeCommand<Commands::Enable>((GLenum)GL_CULL_FACE);
    m_SpotLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_FRONT);

    m_SpotLightQueue->MakeCommand<Commands::Texture2DBindUnit>(0u, 0u);
    m_SpotLightQueue->MakeCommand<Commands::VAOBindResource>(coneMesh.deviceHandle);
    m_SpotLightQueue->MakeCommand<Commands::VAODrawTrianglesBaseVertex>(*garg);

    m_SpotLightQueue->MakeCommand<Commands::CullFace>((GLenum)GL_BACK);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    m_SpotLightQueue->MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);
}

} 
