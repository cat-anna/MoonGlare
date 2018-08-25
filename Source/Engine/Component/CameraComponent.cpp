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

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Foundation/Math/Geometry.h>

namespace MoonGlare::Component {

RegisterComponentID<CameraComponent> CameraComponentReg("Camera");

CameraComponent::CameraComponent(SubsystemManager *Owner)
        : TemplateStandardComponent(Owner) 
        , m_TransformComponent(nullptr) {
}

CameraComponent::~CameraComponent() { }

//------------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer CameraComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
        .beginClass<CameraComponentEntry>("cCameraComponentEntry")
            .addProperty("Active", &CameraComponentEntry::GetActive, &CameraComponentEntry::SetActive)
        .endClass()
        ;
}

//------------------------------------------------------------------------------------------

bool CameraComponent::Initialize() {
    m_Array.fill(CameraComponentEntry());
    //	m_Array.MemZeroAndClear();

    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }

    return true;
}

bool CameraComponent::Finalize() {
    return true;
}

//------------------------------------------------------------------------------------------

void CameraComponent::Step(const Core::MoveConfig & conf) {
    if (m_Array.empty()) {
        return;
    }

    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

    size_t ActiveId = 0xFFFF;
    bool GotActive = false;

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {
        auto &item = m_Array[i];
        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and ignore
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }

        if (!item.m_Flags.m_Map.m_Active) {
            //camera is not active, continue
            continue;
        }

        if (GotActive) {
            if (item.m_Flags.m_Map.m_ActiveChanged) {
                m_Array[ActiveId].m_Flags.m_Map.m_Active = false;
                item.m_Flags.m_Map.m_ActiveChanged = false;
            } else {
                AddLog(Warning, "There is active camera, disabling");
                item.m_Flags.m_Map.m_Active = false;
                continue;
            }
        }

        GotActive = true;
        ActiveId = i;

        auto tcindex = m_TransformComponent->GetComponentIndex(item.m_Owner);
        if (tcindex == ComponentIndex::Invalid) {
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        auto &tr = m_TransformComponent->GetTransform(tcindex);

        //tr.v
        emath::Quaternion up = { 0,0,1,0 };
        auto q = emath::Quaternion(tr.linear());
        emath::Quaternion rotatedP = q * up * q.inverse();

        emath::fvec3 d = rotatedP.vec();
        emath::fvec3 p = tr.translation();

        auto &cam = *conf.deffered->camera;
        cam.m_Position = p; 
        cam.m_Direction = d;

        auto view = emath::LookAt(p, (emath::fvec3)(p - d), emath::fvec3(0, 0, 1));
        cam.m_ProjectionMatrix = item.m_ProjectionMatrix * view;
    }

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "CameraComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        TrivialReleaseElement(LastInvalidEntry);
    }
}

bool CameraComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    x2c::Component::CameraComponent::CameraEntry_t ce;
    ce.ResetToDefault();
    if (!reader.Read(ce)) {
        AddLogf(Error, "Failed to read CameraComponent entry!");
        return false;
    }

    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLog(Error, "Failed to allocate index");
        //no need to deallocate entry. It will be handled by internal garbage collecting mechanism
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();
    entry.m_Owner = owner;

    entry.m_Flags.m_Map.m_Orthogonal = ce.m_Orthogonal;
    entry.m_FoV = ce.m_FoV;

    entry.m_Flags.m_Map.m_Active = ce.m_Active;
    m_EntityMapper.SetIndex(owner, index);

    auto s = GetManager()->GetWorld()->GetRendererFacade()->GetContext()->GetSizef();
    entry.ResetProjectionMatrix(s);
                                
    entry.m_Flags.m_Map.m_Valid = true;
    return true;
}

//-------------------------------------------------------------------------------------------------

void CameraComponentEntry::ResetProjectionMatrix(const emath::fvec2 &ScreenSize) {
    float Aspect = ScreenSize[0] / ScreenSize[1];
    float Near = 0.1f, Far = 1.0e4f;// TODO;
    if (m_Flags.m_Map.m_Orthogonal) {
        m_ProjectionMatrix = emath::Ortho(0.0f, Aspect, 1.0f, 0.0f, Near, Far);
    } else {
        m_ProjectionMatrix = emath::Perspective(m_FoV, Aspect, Near, Far);
    }
}

}
