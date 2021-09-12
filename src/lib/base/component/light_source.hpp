#pragma once

#include "component_common.hpp"
#include "component_serialiazation.hpp"
// #include <Renderer/Light.h>

namespace MoonGlare::Component {

// alignas(16)
struct LightSource : public ComponentBase<LightSource> {
    static constexpr ComponentId kComponentId = 8;
    static constexpr char kComponentName[] = "light_source";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;
    static constexpr bool kHasResources = false;

    // Renderer::LightBase base;
    // Renderer::LightAttenuation attenuation;
    // Renderer::LightType type;
    // float cut_off;
};

// static_assert((sizeof(Light) % 16) == 0);

#ifdef WANTS_TYPE_INFO

inline auto GetTypeInfo(LightSource *) {
    return AttributeMapBuilder<LightSource>::Start(LightSource::kComponentName)
        // ->AddField("scale", &LightSource::scale)
        // ->AddField("position", &LightSource::position)
        // ->AddField()
        // ->AddField()
        // ->AddField()
        ;
}

#endif

#ifdef WANTS_SERIALIZATION

inline void to_json(nlohmann::json &j, const LightSource &p) {
    j = {
        // {"name", p.name},
    };
}
inline void from_json(const nlohmann::json &j, LightSource &p) {
    // j.at("name").get_to(p.name);
}

#endif

} // namespace MoonGlare::Component

#if 0
#include <nfMoonGlare.h>
#include <pch.h>

#include "LightComponent.h"
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/SubsystemManager.h>

#include "TransformComponent.h"

#include <Common.x2c.h>
#include <ComponentCommon.x2c.h>
#include <LightComponent.x2c.h>
#include <Math.x2c.h>

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
#endif
