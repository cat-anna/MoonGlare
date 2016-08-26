/*
  * Generated by cppsrc.sh
  * On 2016-08-06 22:30:28,74
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include <Renderer/VirtualCamera.h>
#include <Renderer/RenderInput.h>

#include "CameraComponent.h"
#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

#include <Common.x2c.h>
#include <ComponentCommon.x2c.h>
#include <CameraComponent.x2c.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

::Space::RTTI::TypeInfoInitializer<CameraComponent, CameraComponentEntry> CameraComponentTypeInfo;
RegisterComponentID<CameraComponent> CameraComponentReg("Camera", true, &CameraComponent::RegisterScriptApi);

CameraComponent::CameraComponent(ComponentManager *Owner)
		: TemplateStandardComponent(Owner) {
}

CameraComponent::~CameraComponent() {

}

//------------------------------------------------------------------------------------------

void CameraComponent::RegisterScriptApi(ApiInitializer & root) {
	root
		.beginClass<CameraComponentEntry>("cCameraComponentEntry")
			.addProperty("Active", &CameraComponentEntry::GetActive, &CameraComponentEntry::SetActive)
		.endClass()
		;
}

//------------------------------------------------------------------------------------------

bool CameraComponent::Initialize() {
	m_Array.fill(CameraComponentEntry());
	//	m_Array.MemZeroAndClear();
	return true;
}

bool CameraComponent::Finalize() {
	return true;
}

//------------------------------------------------------------------------------------------

void CameraComponent::Step(const Core::MoveConfig & conf) {
	if (m_Array.Empty()) {
		return;
	}

	auto *tc = GetManager()->GetTransformComponent();
	auto *RInput = conf.m_RenderInput.get();

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

		if (!GetHandleTable()->IsValid(item.m_SelfHandle)) {
			AddLogf(Error, "CameraComponent: invalid entity at index %d", i);
			item.m_Flags.m_Map.m_Valid = false;
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

		auto *tcentry = tc->GetEntry(item.m_Owner);
		if (!tcentry) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further checks
			continue;
		}

		auto &tr = tcentry->m_GlobalTransform;
		auto q = tr.getRotation();
		auto p = convert(tr.getOrigin());
		auto d = convert(quatRotate(q, Physics::vec3(0, 0, 1)));

		RInput->m_Camera.m_Position = p;
		RInput->m_Camera.m_Direction = d;

		//RInput->m_Camera.UpdateMatrix();
		auto view = glm::lookAt(p, p + d, math::vec3(0, 1, 0));
		RInput->m_Camera.m_WorldMatrix = (math::mat4&)item.m_ProjectionMatrix * view;
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "CameraComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		TrivialReleaseElement(LastInvalidEntry);
	}
}

bool CameraComponent::Load(xml_node node, Entity Owner, Handle & hout) {
	x2c::Component::CameraComponent::CameraEntry_t ce;
	ce.ResetToDefault();
	if (!ce.Read(node)) {
		AddLogf(Error, "Failed to read CameraComponent entry!");
		return false;
	}

	Handle &h = hout;
	HandleIndex index;
	if (!m_Array.Allocate(index)) {
		AddLog(Error, "Failed to allocate index");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();
	if (!GetHandleTable()->Allocate(this, Owner, h, index)) {
		AddLog(Error, "Failed to allocate handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	auto *tc = GetManager()->GetTransformComponent();
	if (!tc->GetInstanceHandle(Owner, entry.m_TransformHandle)) {
		AddLog(Error, "Cannot get handle to TransformComponent instance!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	entry.m_SelfHandle = h;
	entry.m_Owner = Owner;

	entry.m_Flags.m_Map.m_Orthogonal = ce.m_Orthogonal;
	entry.m_FoV = ce.m_FoV;

//	void VirtualCamera::SetDefaultPerspective() {
//		auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
//		SetPerspective(ScreenSize[0] / ScreenSize[1]);
//	}
//	void VirtualCamera::SetDefaultOrthogonal() {
//		auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
//		SetOrthogonal(ScreenSize[0], ScreenSize[1]);
//	}

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Active = ce.m_Active;
	m_EntityMapper.SetHandle(Owner, h);

	entry.ResetProjectionMatrix();

	return true;
}

bool CameraComponent::Create(Entity Owner, Handle & hout) {
	return false;
}

//-------------------------------------------------------------------------------------------------

void CameraComponentEntry::ResetProjectionMatrix() {
	auto ScreenSize = math::fvec2(Graphic::GetRenderDevice()->GetContextSize());
	float Aspect = ScreenSize[0] / ScreenSize[1];
	if (m_Flags.m_Map.m_Orthogonal) {
		m_ProjectionMatrix = glm::ortho(0.0f, Aspect, 1.0f, 0.0f);

		//	void VirtualCamera::SetOrthogonal(float Width, float Height) {
		//		m_UseViewMatrix = false;
		//		UpdateMatrix();
		//		m_WorldMatrix = m_ProjectionMatrix;
		//	}
	} else {
		float Near = 0.1f, Far = 1.0e4f;// TODO;
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FoV), Aspect, Near, Far);

		//	void VirtualCamera::SetPerspective(float Aspect, float FoV, float Near, float Far) {
		//		m_UseViewMatrix = true;
		//		UpdateMatrix();
		//	}
	}
}

} //namespace Component 
} //namespace Renderer 
} //namespace MoonGlare 