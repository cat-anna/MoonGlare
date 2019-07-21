#include <pch.h>
#include <nfMoonGlare.h>

#include "LightSystem.h"
#include <Core/Component/SubsystemManager.h>

#include "TransformComponent.h"
#include "LightComponent.h"

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Math/Geometry.h>

namespace MoonGlare::Component {

LightSystem::LightSystem(iSubsystemManager * Owner)
        : subsystemManager(Owner)  {
}

LightSystem::~LightSystem() {
}

//------------------------------------------------------------------------------------------

bool LightSystem::Initialize() {
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

void LightSystem::Step(const SubsystemUpdateData& xconf) {
    const Core::MoveConfig &conf = (const Core::MoveConfig &)xconf;

    componentArray->Visit<LightComponent>([this, &conf](Entity owner, LightComponent& item) {
        //TODO: visibility test
        auto tindex = transformComponent->GetComponentIndex(owner);
        if (tindex == ComponentIndex::Invalid) {
            //shall not happen
            return;
        }

        auto &tr = transformComponent->GetTransform(tindex);
        auto &globalScale = transformComponent->GetGlobalScale(tindex);
        auto maxScale = 1.0f / globalScale.maxCoeff();

        switch (item.type) {
        case Renderer::LightType::Spot:
        {
            Renderer::SpotLight sl;
            sl.m_Base = item.base;
            sl.m_Attenuation = item.attenuation;
            sl.m_CutOff = item.cutOff;

            //TODO: SpotLigt calculations can be optimized later

            //emath::Quaternion rotatedP = q * up * q.inverse();
            //emath::fvec3 d = rotatedP.vec();

            emath::fvec4 dir4 = tr * emath::fvec4(0, 1, 0, 0);
            emath::fvec3 dir = { dir4.x(), dir4.y(), dir4.z() };

            //auto dir = convert(quatRotate(tr.getRotation(), Physics::vec3(0, 0, 1)));
            emath::fvec3 pos = tr.translation();// convert(tr.getOrigin());
            float infl = sl.GetLightInfluenceRadius(maxScale);

            emath::fmat4 ViewMatrix = emath::LookAt(pos, emath::fvec3(pos - dir), emath::fvec3(0, 0, 1));// glm::lookAt(pos, pos - dir, math::vec3(0, 1, 0));
            emath::fmat4 ProjectionMatrix = emath::Perspective(90.0f, 1.0f, 0.1f, infl + 0.1f);

            auto scaled = tr;
            scaled.scale(infl * maxScale);

            sl.m_PositionMatrix = scaled.data();
            sl.m_ViewMatrix = ProjectionMatrix * ViewMatrix;
            sl.m_Position = pos;
            sl.m_Direction = dir;

            conf.deffered->SubmitSpotLight(sl);
            return;
        }
        case Renderer::LightType::Point:
        {
            Renderer::PointLight pl;
            pl.m_Base = item.base;
            pl.m_Attenuation = item.attenuation;
            float infl = pl.GetLightInfluenceRadius(maxScale);

            emath::fvec3 pos = tr.translation();
            pl.m_Position = pos.data();// convert(tr.getOrigin());
            //if (!conf.deffered->PointLightVisibilityTest(pos, infl))
                //continue;

            auto scaled = tr;
            scaled.scale(infl * maxScale);

            pl.m_PositionMatrix = scaled.data();

            conf.deffered->SubmitPointLight(pl);
            return;
        }
        case Renderer::LightType::Directional:
            conf.deffered->SubmitDirectionalLight(item.base);
            return;
        default:
            return;
        }
    });
}

} 
