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

    void Mesh(const emath::fmat4 &ModelMatrix, MeshResourceHandle meshH, MaterialResourceHandle matH, bool castShadow);

    struct DrawInfo {
        unsigned numIndices;
        unsigned baseVertex;
        unsigned baseIndex;
        uint16_t indexElementType;
        uint16_t elementMode;
    };

    void SetCamera(const VirtualCamera &camera);

    void DrawElements(const emath::fmat4 &ModelMatrix, VAOResourceHandle vao, DrawInfo info, MaterialResourceHandle matH, bool castShadow);

    void SubmitDirectionalLight(const LightBase &linfo);
    void SubmitPointLight(const PointLight &linfo);
    void SubmitSpotLight(const SpotLight &linfo);

    bool PointLightVisibilityTest(const emath::fvec3 &position, float radius);

    void SetStaticFog(const StaticFog &fog);

    emath::fvec2 GetScreenSize() const;
    float GetAspect() const {
        auto ss = GetScreenSize();
        return  ss.x() / ss.y();
    }
protected:
    std::unique_ptr<DeferredSink> sink;
};

} 
