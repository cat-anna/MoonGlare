#include "DeferredFrontend.h"
#include "DeferredPipeline.h"

namespace MoonGlare::Renderer::Deferred {

DefferedFrontend::DefferedFrontend() {
    sink = std::make_unique<DeferredSink>();
}

DefferedFrontend::~DefferedFrontend() { }

void DefferedFrontend::Reset(Frame *frame) {
    camera = &sink->m_Camera;
    sink->Reset(frame);
}

void DefferedFrontend::Initialize(RendererFacade *Renderer) {
    sink->Initialize(Renderer);
}

void DefferedFrontend::Mesh(const emath::fmat4 &ModelMatrix, const emath::fvec3 &basepos, Renderer::MeshResourceHandle meshH, MaterialResourceHandle matH) {
    sink->Mesh(ModelMatrix, basepos, meshH, matH);
}

void DefferedFrontend::SubmitDirectionalLight(const LightBase &linfo) {
    sink->SubmitDirectionalLight(linfo);
}
void DefferedFrontend::SubmitPointLight(const PointLight &linfo) {
    sink->SubmitPointLight(linfo);
}
void DefferedFrontend::SubmitSpotLight(const SpotLight &linfo) {
    sink->SubmitSpotLight(linfo);
}

void DefferedFrontend::SetStaticFog(const StaticFog &fog) {
    sink->SetStaticFog(fog);
}

bool DefferedFrontend::PointLightVisibilityTest(const emath::fvec3 &position, float radius) {
    return sink->PointLightVisibilityTest(position, radius);
}

} 
