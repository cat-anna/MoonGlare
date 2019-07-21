#include <pch.h>
#include <nfMoonGlare.h>

#include "CameraSystem.h"
#include "CameraComponent.h"
#include "TransformComponent.h"

#include <Core/Component/SubsystemManager.h>

#include <Renderer/Renderer.h>
#include <Renderer/iContext.h>
#include <Renderer/Deferred/DeferredFrontend.h>

#include <Common.x2c.h>
#include <ComponentCommon.x2c.h>
#include <CameraComponent.x2c.h>

#include <Math/Geometry.h>

namespace MoonGlare::Component {

CameraSystem::CameraSystem(iSubsystemManager *Owner)
    : subsystemManager(Owner) {
}

CameraSystem::~CameraSystem() { }

//------------------------------------------------------------------------------------------

bool CameraSystem::Initialize() {
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

void CameraSystem::Step(const SubsystemUpdateData &xconf) {

    if (gotActiveCamera) {
        if (SubmitCamera(activeCamera, xconf))
            return;
        gotActiveCamera = false;
    }

    if (!gotActiveCamera) {
        bool foundActive = false;
        componentArray->Visit<CameraComponent>([this, &xconf, &foundActive](Entity owner, CameraComponent& item) {
            if (foundActive) {
                componentArray->SetActive<CameraComponent>(owner, false);
            } else {
                activeCamera = owner;

                const Core::MoveConfig &conf = (const Core::MoveConfig &)xconf;
                item.ResetProjectionMatrix(conf.deffered->GetAspect());

                foundActive = SubmitCamera(owner, xconf);
                gotActiveCamera = foundActive;
            }
        });
    }
}

bool CameraSystem::SubmitCamera(Entity cameraEntity, const SubsystemUpdateData &xconf) {
    const Core::MoveConfig &conf = (const Core::MoveConfig &)xconf;

    auto tcindex = transformComponent->GetComponentIndex(cameraEntity);
    if (tcindex == ComponentIndex::Invalid) {
        //should not happen
        return false;
    }

    auto *cptr = componentArray->Get<CameraComponent>(cameraEntity);
    if (!cptr)
        return false;
    auto &tr = transformComponent->GetTransform(tcindex);

    static const emath::Quaternion up = { 0,0,1,0 };//for e-math w is first arg
    auto q = emath::Quaternion(tr.linear());
    emath::Quaternion rotatedP = q * up * q.inverse();

    emath::fvec3 d = rotatedP.vec();
    emath::fvec3 p = tr.translation();

    Renderer::VirtualCamera cam;
    cam.m_Position = p;
    cam.m_Direction = d;

    auto view = emath::LookAt(p, (emath::fvec3)(p - d), emath::fvec3(0, 0, 1));
    cam.m_ProjectionMatrix = cptr->m_ProjectionMatrix * view;

    conf.deffered->SetCamera(cam);

    return true;
}

void CameraSystem::ActivateCamera(Entity cameraOwner) {
    if (gotActiveCamera) {
        componentArray->SetActive<CameraComponent>(activeCamera, false);
    }
    activeCamera = cameraOwner;
}

}
