#include <pch.h>
#include <nfMoonGlare.h>

#include "CameraComponent.h"
#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include "TransformComponent.h"

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/iContext.h>

#include <Common.x2c.h>
#include <ComponentCommon.x2c.h>
#include <CameraComponent.x2c.h>

#include <Foundation/Math/Geometry.h>

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
