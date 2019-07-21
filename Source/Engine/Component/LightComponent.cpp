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

#include <Math/Geometry.h>

namespace MoonGlare::Component {

bool LightComponent::Load(ComponentReader &reader, Entity owner) {
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
    
    base.m_AmbientIntensity = le.m_AmbientIntensity;
    base.m_DiffuseIntensity = le.m_DiffuseIntensity;
    base.m_Color = le.m_Color;
    base.m_Flags.m_CastShadows = le.m_CastShadows;
    attenuation.values = le.m_Attenuation;
    type = le.m_Type;
    cutOff = le.m_CutOff;
    
    return true;
}

} 
