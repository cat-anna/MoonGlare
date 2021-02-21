#pragma once

#include "component_common.hpp"
#include "component_serialiazation.hpp"
#include "math/vector.hpp"
// #include <Renderer/VirtualCamera.h>

namespace MoonGlare::Component {

// alignas(16)
struct Camera : public ComponentBase<Camera> {
    static constexpr ComponentId kComponentId = 7;
    static constexpr char kComponentName[] = "camera";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    math::fmat4 projection_matrix;
    bool orthogonal;
    uint8_t _padding_0[3];
    float fov;
};

// static_assert((sizeof(Camera) % 16) == 0);
static_assert((offsetof(Camera, projection_matrix) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(Camera *) {
    return AttributeMapBuilder<Camera>::Start(Camera::kComponentName)
        ->AddField("orthogonal", &Camera::orthogonal)
        ->AddField("fov", &Camera::fov);
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const Camera &p) {
    j = {
        {"orthogonal", p.orthogonal},
        {"fov", p.fov},
    };
}
void from_json(const nlohmann::json &j, Camera &p) {
    j.at("orthogonal").get_to(p.orthogonal);
    j.at("fov").get_to(p.fov);
}

#endif

} // namespace MoonGlare::Component

#if 0

#include <nfMoonGlare.h>
#include <pch.h>

#include "CameraComponent.h"
#include "TransformComponent.h"
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/SubsystemManager.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/iContext.h>

#include <CameraComponent.x2c.h>
#include <Common.x2c.h>
#include <ComponentCommon.x2c.h>

#include <Math/Geometry.h>

namespace MoonGlare::Component {

bool CameraComponent::Load(ComponentReader &reader, Entity owner) {
    x2c::Component::CameraComponent::CameraEntry_t ce;
    ce.ResetToDefault();
    if (!reader.Read(ce)) {
        AddLogf(Error, "Failed to read CameraComponent entry!");
        return false;
    }

    m_Flags.ClearAll();

    m_Flags.m_Map.m_Orthogonal = ce.m_Orthogonal;
    m_FoV = ce.m_FoV;
    m_ProjectionMatrix.setIdentity();

    return true;
}

//-------------------------------------------------------------------------------------------------

void CameraComponent::ResetProjectionMatrix(float aspect) {
    float Near = 0.1f, Far = 1.0e4f;// TODO;
    if (m_Flags.m_Map.m_Orthogonal) {
        m_ProjectionMatrix = emath::Ortho(0.0f, aspect, 1.0f, 0.0f, Near, Far);
    } else {
        m_ProjectionMatrix = emath::Perspective(m_FoV, aspect, Near, Far);
    }
}

}

#endif
