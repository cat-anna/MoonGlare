#include <pch.h>
#include <nfMoonGlare.h>

#include "LightComponent.h"
#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>

#include "TransformComponent.h"

#include <Common.x2c.h>
#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <LightComponent.x2c.h>

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Foundation/Math/Geometry.h>

namespace MoonGlare::Component {

RegisterComponentID<LightComponent> LightComponentReg("Light");

LightComponent::LightComponent(SubsystemManager * Owner) 
        : TemplateStandardComponent(Owner) 
        , m_TransformComponent(nullptr) {
}

LightComponent::~LightComponent() {
}

//------------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer LightComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    using LightAttenuation = Renderer::LightAttenuation;
    return root
        .beginClass<LightAttenuation>("cLightAttenuation")
            .addProperty("Constant", &LightAttenuation::Constant, &LightAttenuation::SetConstant )
            .addProperty("Linear", &LightAttenuation::Linear, &LightAttenuation::SetLinear)
            .addProperty("Exp", &LightAttenuation::Exp, &LightAttenuation::SetExp)
            .addProperty("Threshold", &LightAttenuation::Threshold, &LightAttenuation::SetThreshold)
        .endClass()
        .beginClass<LightEntry>("cLightEntry")
            .addData("CutOff", &LightEntry::m_CutOff)
            .addData("Attenuation", &LightEntry::m_Attenuation)
            //.addData("Color", &LightEntry::m_Color)
            //.addData("AmbientIntensity", &LightEntry::m_AmbientIntensity)
            //.addData("DiffuseIntensity", &LightEntry::m_DiffuseIntensity)
            .addProperty("Active", &LightEntry::GetActive, &LightEntry::SetActive)
            .addProperty("CastShadows", &LightEntry::GetCastShadows, &LightEntry::SetCastShadows)
        .endClass()
    ;
}

//------------------------------------------------------------------------------------------

bool LightComponent::Initialize() {
    m_Array.fill(LightEntry());

    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }

    return true;
}

bool LightComponent::Finalize() {
    return true;
}

//------------------------------------------------------------------------------------------

void LightComponent::Step(const Core::MoveConfig & conf) {
    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
        auto &item = m_Array[i];
    
        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and continue
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }
    
        if (!item.m_Flags.m_Map.m_Active) {
            continue;
        }
    
        auto tcindex= m_TransformComponent->GetComponentIndex(item.m_Owner);
        if (tcindex == ComponentIndex::Invalid) {
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        const auto &tr = m_TransformComponent->GetTransform(tcindex);

        switch (item.m_Type) {
        case Renderer::LightType::Spot: {
            Renderer::SpotLight sl;
            sl.m_Base = item.m_Base;
            sl.m_Attenuation = item.m_Attenuation;
            sl.m_CutOff = item.m_CutOff;

            //TODO: SpotLigt calculations can be optimized later

            //emath::Quaternion rotatedP = q * up * q.inverse();
            //emath::fvec3 d = rotatedP.vec();

            emath::fvec4 dir4 = tr * emath::fvec4(0, 0, 1, 0);
            emath::fvec3 dir = { dir4.x(), dir4.y(), dir4.z() };

            //auto dir = convert(quatRotate(tr.getRotation(), Physics::vec3(0, 0, 1)));
            emath::fvec3 pos = tr.translation();// convert(tr.getOrigin());
            float infl = sl.GetLightInfluenceRadius();

            emath::fmat4 ViewMatrix = emath::LookAt(pos, emath::fvec3(pos - dir), emath::fvec3(0, 1, 0));// glm::lookAt(pos, pos - dir, math::vec3(0, 1, 0));
            emath::fmat4 ProjectionMatrix = emath::Perspective(90.0f, 1.0f, 0.01f, infl + 0.1f);

            auto scaled = tr;
            scaled.scale(infl);

            sl.m_PositionMatrix = scaled.data();
            sl.m_ViewMatrix = ProjectionMatrix * ViewMatrix;
            sl.m_Position = pos;
            sl.m_Direction = dir;

            conf.deffered->SubmitSpotLight(sl);
            continue;
        }
        case Renderer::LightType::Point: {
            Renderer::PointLight pl;
            pl.m_Base = item.m_Base;
            pl.m_Attenuation = item.m_Attenuation;
            float infl = pl.GetLightInfluenceRadius();

            emath::fvec3 pos = tr.translation();
            pl.m_Position = pos.data();// convert(tr.getOrigin());
            if (!conf.deffered->PointLightVisibilityTest(pos, infl))
                continue;

            auto scaled = tr;
            scaled.scale(infl);

            pl.m_PositionMatrix = scaled.data();

            conf.deffered->SubmitPointLight(pl);
            continue;
        }
        case Renderer::LightType::Directional:
            conf.deffered->SubmitDirectionalLight(item.m_Base);
            continue;
        default:
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //entry is not valid and shall be removed
            continue;
        }
    }

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "MeshComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        TrivialReleaseElement(LastInvalidEntry);
    }
}

bool LightComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    x2c::Component::LightComponent::LightEntry_t le;
    le.ResetToDefault();
    if (!reader.Read(le)) {
        AddLogf(Error, "Failed to read LightComponent entry!");
        return false;
    }

    switch (le.m_Type) {
    case Renderer::LightType::Spot:
    case Renderer::LightType::Point:
    case Renderer::LightType::Directional:
        break;
    default:
        AddLogf(Error, "Invalid light type!");
        LogInvalidEnum(le.m_Type);
        return false;
    }

    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();

    entry.m_Owner = owner;

    entry.m_Base.m_AmbientIntensity = le.m_AmbientIntensity;
    entry.m_Base.m_DiffuseIntensity = le.m_DiffuseIntensity;
    entry.m_Base.m_Color = le.m_Color;
    entry.m_Attenuation.values = le.m_Attenuation;
    entry.m_Type = le.m_Type;
    entry.m_CutOff = le.m_CutOff;

    entry.m_Flags.m_Map.m_Active = le.m_Active;
    entry.m_Base.m_Flags.m_CastShadows = le.m_CastShadows;

    m_EntityMapper.SetIndex(entry.m_Owner, index);
    entry.m_Flags.m_Map.m_Valid = true;
    return true;
}

} 
