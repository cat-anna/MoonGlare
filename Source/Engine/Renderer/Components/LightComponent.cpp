/*
  * Generated by cppsrc.sh
  * On 2016-08-21 12:19:52,68
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include "LightComponent.h"
#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

#include <Common.x2c.h>
#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <LightComponent.x2c.h>

#include <Engine/Renderer/Dereferred/DereferredPipeline.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

::Space::RTTI::TypeInfoInitializer<LightComponent, LightComponentEntry, Light::LightBase, Light::PointLight, Light::SpotLight, Light::DirectionalLight, Light::LightAttenuation> LightComponentTypeInfo;
RegisterComponentID<LightComponent> LightComponentReg("Light", true, &LightComponent::RegisterScriptApi);

LightComponent::LightComponent(SubsystemManager * Owner) 
        : TemplateStandardComponent(Owner) 
        , m_TransformComponent(nullptr) {
}

LightComponent::~LightComponent() {
}

//------------------------------------------------------------------------------------------

void LightComponent::RegisterScriptApi(ApiInitializer & root) {
    using LightAttenuation = Renderer::Light::LightAttenuation;
    root
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
    
        if (!GetHandleTable()->IsValid(this, item.m_SelfHandle)) {
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        if (!item.m_Flags.m_Map.m_Active) {
            continue;
        }
    
        auto *tcentry = m_TransformComponent->GetEntry(item.m_Owner);
        if (!tcentry) {
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        auto &tr = tcentry->m_GlobalTransform;

        switch (item.m_Type) {
        case Light::LightType::Spot: {
            Light::SpotLight sl;
            sl.m_Base = item.m_Base;
            sl.m_Attenuation = item.m_Attenuation;
            sl.m_CutOff = item.m_CutOff;

            //TODO: SpotLigt calculations can be optimized later

            auto dir = convert(quatRotate(tr.getRotation(), Physics::vec3(0, 0, 1)));
            auto pos = convert(tr.getOrigin());
            float infl = sl.GetLightInfluenceRadius();

            math::mat4 ViewMatrix = glm::lookAt(pos, pos - dir, math::vec3(0, 1, 0));
            math::mat4 ProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, infl + 0.1f);
            math::mat4 mat;
            tr.getOpenGLMatrix(&mat[0][0]);

            sl.m_PositionMatrix = glm::scale(mat, math::vec3(infl));
            sl.m_ViewMatrix = ProjectionMatrix * ViewMatrix;
            sl.m_Position = pos;
            sl.m_Direction = dir;

            conf.deferredSink->SubmitSpotLight(sl);
            continue;
        }
        case Light::LightType::Point: {

            Light::PointLight pl;
            pl.m_Base = item.m_Base;
            pl.m_Attenuation = item.m_Attenuation;
            float infl = pl.GetLightInfluenceRadius();
            pl.m_Position = convert(tr.getOrigin());
            if (!conf.deferredSink->PointLightVisibilityTest(emath::MathCast<emath::fvec3>((math::fvec3)pl.m_Position), infl))
                continue;

            math::mat4 mat;
            tr.getOpenGLMatrix(&mat[0][0]);
            pl.m_PositionMatrix = glm::scale(mat, math::vec3(infl));

            conf.deferredSink->SubmitPointLight(pl);
            continue;
        }
        case Light::LightType::Directional:	
            conf.deferredSink->SubmitDirectionalLight(item.m_Base);
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

bool LightComponent::Load(xml_node node, Entity Owner, Handle & hout) {
    x2c::Component::LightComponent::LightEntry_t le;
    le.ResetToDefault();
    if (!le.Read(node)) {
        AddLogf(Error, "Failed to read LightComponent entry!");
        return false;
    }

    switch (le.m_Type) {
    case Light::LightType::Spot:
    case Light::LightType::Point:
    case Light::LightType::Directional:
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

    Handle &ch = hout;
    if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
        AddLogf(Error, "Failed to allocate handle!");
        //no need to deallocate entry. It will be handled by internal garbage collecting mechanism
        return false;
    }

    entry.m_Owner = Owner;
    entry.m_SelfHandle = ch;

    entry.m_Base.m_AmbientIntensity = le.m_AmbientIntensity;
    entry.m_Base.m_DiffuseIntensity = le.m_DiffuseIntensity;
    entry.m_Base.m_Color = le.m_Color;
    entry.m_Attenuation.values = le.m_Attenuation;
    entry.m_Type = le.m_Type;
    entry.m_CutOff = le.m_CutOff;

    entry.m_Flags.m_Map.m_Active = le.m_Active;
    entry.m_Base.m_Flags.m_CastShadows = le.m_CastShadows;

    m_EntityMapper.SetHandle(entry.m_Owner, ch);
    entry.m_Flags.m_Map.m_Valid = true;
    return true;
}

bool LightComponent::Create(Entity Owner, Handle & hout) {
    return false;
}

} //namespace Component 
} //namespace Renderer 
} //namespace MoonGlare 
