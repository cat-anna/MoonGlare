#pragma once

#include "../Resources/ResourceManager.h"
#include "../Light.h"
#include "../VirtualCamera.h"
#include "../StaticFog.h"

namespace MoonGlare::Renderer::Deferred {

struct DeferredSink;

struct DefferedFrontend {
    DefferedFrontend();
    ~DefferedFrontend();

    void Reset(Frame *frame);
    void Initialize(RendererFacade *Renderer);

    void Mesh(const emath::fmat4 &ModelMatrix, const emath::fvec3 &basepos, MeshResourceHandle meshH, MaterialResourceHandle matH);

    void SubmitDirectionalLight(const LightBase &linfo);
    void SubmitPointLight(const PointLight &linfo);
    void SubmitSpotLight(const SpotLight &linfo);

    bool PointLightVisibilityTest(const emath::fvec3 &position, float radius);

    void SetStaticFog(const StaticFog &fog);

    VirtualCamera *camera = nullptr;
protected:
    std::unique_ptr<DeferredSink> sink;
};

} 
