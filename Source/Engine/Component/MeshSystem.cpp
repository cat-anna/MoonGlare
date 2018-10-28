#include <pch.h>
#include <nfMoonGlare.h>

#include "MeshSystem.h"
#include "MeshComponent.h"

#include "TransformComponent.h"

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Renderer.h>
#include <Renderer/Frame.h>

#include <Core/Component/SubsystemManager.h>

namespace MoonGlare::Component {

MeshSystem::MeshSystem(iSubsystemManager * Owner) : subsystemManager(Owner) {}

MeshSystem::~MeshSystem() {}

//------------------------------------------------------------------------------------------

bool MeshSystem::Initialize() {
    auto *ssm = (Core::Component::SubsystemManager*)subsystemManager;
    transformComponent = ssm->GetComponent<TransformComponent>();
    if (!transformComponent) {
        AddLog(Error, "Failed to get TransformComponent instance!");
        return false;
    }
    componentArray = &subsystemManager->GetComponentArray();
    return true;
}

//------------------------------------------------------------------------------------------

void MeshSystem::Step(const SubsystemUpdateData &xconf) {
    auto *ssm = (Core::Component::SubsystemManager*)subsystemManager;
    auto *rf = ssm->GetWorld()->GetRendererFacade();
    const Core::MoveConfig &conf = (const Core::MoveConfig &)xconf;
    auto &layers = conf.m_BufferFrame->GetCommandLayers();
    auto &m = conf.m_BufferFrame->GetMemory();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::Controll>();
    auto &q = Queue;

    componentArray->Visit<MeshComponent>([this, &conf, &q, &m, rf, ssm](Entity owner, MeshComponent& item) {
        //TODO: visibility test
        auto tindex = transformComponent->GetComponentIndex(owner);
        if (tindex == ComponentIndex::Invalid) {
            //shall not happen
            return;
        }

        auto &tr = transformComponent->GetTransform(tindex);
        if (item.meshHandle.deviceHandle) {//dirty valid check
            conf.deffered->Mesh(
                tr.matrix(),
                item.meshHandle,
                item.materialHandle,
                item.castShadow
            );
        }
    });
}

}
