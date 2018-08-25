#pragma once

//#include "DeferredFrameBuffer.h"

#include "GeometryShader.h"
#include "PointLightShader.h"
#include "DirectionalLightShader.h"
#include "SpotLightShader.h"
#include "StencilShader.h"
#include "../PlaneShadowMapShader.h"
#include "DeferredFrameBuffer.h"

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Light.h>
#include <Renderer/VirtualCamera.h>
#include <Renderer/StaticFog.h>

#include <Renderer/Commands/CommandQueue.h>

namespace MoonGlare::Renderer::Deferred {

struct DeferredSink {
    friend class DeferredPipeline;

    DeferredSink();

    void Reset(Frame *frame);
    void Initialize(RendererFacade *Renderer);

    bool MeshVisibilityTest(const emath::fvec3 &position, float radius);
    void Mesh(const emath::fmat4 &ModelMatrix, const emath::fvec3 &basepos, MeshResourceHandle meshH, MaterialResourceHandle matH);

    void SubmitDirectionalLight(const LightBase &linfo);
    void SubmitPointLight(const PointLight &linfo);
    bool PointLightVisibilityTest(const emath::fvec3 &position, float radius);
    void SubmitSpotLight(const SpotLight &linfo);

    void SetStaticFog(const StaticFog &afog);
    VirtualCamera m_Camera;
protected:
    DeferredFrameBuffer m_Buffer;
    emath::fvec2 m_ScreenSize;

    Commands::CommandQueue *m_GeometryQueue = nullptr;
    Commands::CommandQueue *m_LightGeometryQueue = nullptr;
    Commands::CommandQueue *m_DirectionalLightQueue = nullptr;
    Commands::CommandQueue *m_PointLightQueue = nullptr;
    Commands::CommandQueue *m_SpotLightQueue = nullptr;
    Commands::CommandQueue *m_SpotLightShadowQueue = nullptr;

    Resources::Shader::ShaderBuilder<GeometryShaderDescriptor> m_GeometryShader;
    Resources::Shader::ShaderBuilder<PlaneShadowMapShaderDescriptor> m_ShadowShader;
    Resources::Shader::ShaderBuilder<DirectionalLightShaderDescriptor> m_DirectionalLightShader;
    Resources::Shader::ShaderBuilder<PointLightShaderDescriptor> m_PointLightShader;
    Resources::Shader::ShaderBuilder<StencilLightShaderDescriptor> m_StencilShader;
    Resources::Shader::ShaderBuilder<SpotLightShaderDescriptor> m_SpotShader;

    ShaderResourceHandle<PlaneShadowMapShaderDescriptor> m_ShaderShadowMapHandle{};
    ShaderResourceHandle<SpotLightShaderDescriptor> m_ShaderLightSpotHandle{};
    ShaderResourceHandle<PointLightShaderDescriptor> m_ShaderLightPointHandle{};
    ShaderResourceHandle<DirectionalLightShaderDescriptor> m_ShaderLightDirectionalHandle{};
    ShaderResourceHandle<StencilLightShaderDescriptor> m_ShaderStencilHandle{};
    ShaderResourceHandle<GeometryShaderDescriptor> m_ShaderGeometryHandle{};

    uint32_t meshcouter = 0;
    float visibility = -1.0f;

    Frame *m_frame = nullptr;
    RendererFacade *m_Renderer = nullptr;
    DeferredPipeline *m_DeferredPipeline;

    MeshResourceHandle sphereMesh;
    MeshResourceHandle coneMesh;
    MeshResourceHandle quadMesh;

    Renderer::StaticFog fog = {};
    bool fogSet = false;

    void InitializeDirectionalQuad();
};

} 
