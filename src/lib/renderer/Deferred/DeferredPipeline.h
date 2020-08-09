#pragma once

#include "../CubeShadowMapShader.h"
#include "../PlaneShadowMapShader.h"
#include "../PostProcessShader.h"
#include "DeferredFrameBuffer.h"
#include "DirectionalLightShader.h"
#include "GeometryShader.h"
#include "PointLightShader.h"
#include "SpotLightShader.h"
#include "StencilShader.h"
#include <Light.h>
#include <Resources/ResourceManager.h>
#include <Resources/Shader/ShaderBuilder.h>
#include <StaticFog.h>
#include <VirtualCamera.h>

#include <Commands/CommandQueue.h>

namespace MoonGlare::Renderer::Deferred {

struct DeferredSink {
    friend class DeferredPipeline;

    DeferredSink();

    void Reset(Frame *frame);
    void Initialize(RendererFacade *Renderer);

    void SetCamera(const VirtualCamera &camera);

    bool MeshVisibilityTest(const emath::fvec3 &position, float radius);
    void Mesh(const emath::fmat4 &ModelMatrix, MeshResourceHandle meshH, MaterialResourceHandle matH, bool castShadow);

    void DrawElements(const emath::fmat4 &ModelMatrix, VAOResourceHandle vao, DefferedFrontend::DrawInfo info,
                      MaterialResourceHandle matH, bool castShadow);

    void SubmitDirectionalLight(const LightBase &linfo);
    void SubmitPointLight(const PointLight &linfo);
    bool PointLightVisibilityTest(const emath::fvec3 &position, float radius);
    void SubmitSpotLight(const SpotLight &linfo);

    void SetStaticFog(const StaticFog &afog);

    emath::fvec2 GetScreenSize() const { return m_ScreenSize; }

protected:
    VirtualCamera m_Camera;
    DeferredFrameBuffer m_Buffer;
    emath::fvec2 m_ScreenSize;
    uint16_t shadowMapSize = 0;

    Commands::CommandQueue *m_GeometryQueue = nullptr;
    Commands::CommandQueue *m_LightGeometryQueue = nullptr;
    Commands::CommandQueue *m_CubeLightGeometryQueue = nullptr;
    Commands::CommandQueue *m_DirectionalLightQueue = nullptr;
    Commands::CommandQueue *m_PointLightQueue = nullptr;
    Commands::CommandQueue *m_PointLightShadowQueue = nullptr;
    Commands::CommandQueue *m_SpotLightQueue = nullptr;
    Commands::CommandQueue *m_SpotLightShadowQueue = nullptr;

    Resources::Shader::ShaderBuilder<GeometryShaderDescriptor> m_GeometryShader;
    Resources::Shader::ShaderBuilder<PlaneShadowMapShaderDescriptor> m_PlaneShadowShader;
    Resources::Shader::ShaderBuilder<CubeShadowMapShaderDescriptor> m_CubeShadowShader;
    Resources::Shader::ShaderBuilder<DirectionalLightShaderDescriptor> m_DirectionalLightShader;
    Resources::Shader::ShaderBuilder<PointLightShaderDescriptor> m_PointLightShader;
    Resources::Shader::ShaderBuilder<StencilLightShaderDescriptor> m_StencilShader;
    Resources::Shader::ShaderBuilder<SpotLightShaderDescriptor> m_SpotShader;

    ShaderResourceHandle<PlaneShadowMapShaderDescriptor> m_PlaneShaderShadowMapHandle{};
    ShaderResourceHandle<CubeShadowMapShaderDescriptor> m_CubeShaderShadowMapHandle{};
    ShaderResourceHandle<SpotLightShaderDescriptor> m_ShaderLightSpotHandle{};
    ShaderResourceHandle<PointLightShaderDescriptor> m_ShaderLightPointHandle{};
    ShaderResourceHandle<DirectionalLightShaderDescriptor> m_ShaderLightDirectionalHandle{};
    ShaderResourceHandle<StencilLightShaderDescriptor> m_ShaderStencilHandle{};
    ShaderResourceHandle<GeometryShaderDescriptor> m_ShaderGeometryHandle{};
    ShaderResourceHandle<PostProcessShader> m_PostProcessShaderHandle{};

    uint32_t meshcouter = 0;
    float visibility = -1.0f;

    Frame *m_frame = nullptr;
    RendererFacade *m_Renderer = nullptr;
    DeferredPipeline *m_DeferredPipeline;

    MeshResourceHandle sphereMesh;
    MeshResourceHandle coneMesh;
    MeshResourceHandle quadMesh;
    // MeshResourceHandle doomMesh;

    Renderer::StaticFog fog = {};

    void InitializeDirectionalQuad();
};

} // namespace MoonGlare::Renderer::Deferred
