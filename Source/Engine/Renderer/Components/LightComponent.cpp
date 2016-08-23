/*
  * Generated by cppsrc.sh
  * On 2016-08-21 12:19:52,68
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include <Renderer/RenderInput.h>

#include "LightComponent.h"
#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

#include <Common.x2c.h>
#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <LightComponent.x2c.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

RegisterComponentID<LightComponent> LightComponentReg("Light", true, &LightComponent::RegisterScriptApi);

LightComponent::LightComponent(ComponentManager * Owner) 
		: TemplateStandardComponent(Owner) {
}

LightComponent::~LightComponent() {
}

//------------------------------------------------------------------------------------------

void LightComponent::RegisterScriptApi(ApiInitializer & root) {
	using LightAttenuation = Renderer::Light::LightAttenuation;
	root
		.beginClass<LightAttenuation>("cLightAttenuation")
			.addData("Constant", &LightAttenuation::m_Constant)
			.addData("Linear", &LightAttenuation::m_Linear)
			.addData("Exp", &LightAttenuation::m_Exp)
			.addData("Threshold", &LightAttenuation::m_Threshold)
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
	return true;
}

bool LightComponent::Finalize() {
	return true;
}

//------------------------------------------------------------------------------------------

void LightComponent::Step(const Core::MoveConfig & conf) {
	auto *tc = GetManager()->GetTransformComponent();
	auto *RInput = conf.m_RenderInput.get();

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
	
		auto *tcentry = tc->GetEntry(item.m_Owner);
		if (!tcentry) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further checks
			continue;
		}

		auto &tr = tcentry->m_GlobalTransform;

		Renderer::Light::LightBase *light;
		switch (item.m_Type) {
		case Light::LightType::Spot: {
			auto lptr = RInput->m_SpotLights.Allocate();
			if (!lptr) {
				AddLog(Warning, "Cannot allocate SpotLights!");
				continue;
			}
			light = &lptr->m_Base;
			lptr->m_Attenuation = item.m_Attenuation;
			lptr->m_CutOff = item.m_CutOff;

			//TODO: SpotLigt calculations can be optimized later
			auto dir = convert(quatRotate(tr.getRotation(), Physics::vec3(0, 0, 1)));
			auto pos = convert(tr.getOrigin());
			float infl = lptr->GetLightInfluenceRadius();
			math::mat4 ViewMatrix = glm::lookAt(pos, pos - dir, math::vec3(0, 1, 0));
			math::mat4 ProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, infl + 0.1f);

			lptr->m_ViewMatrix = ProjectionMatrix * ViewMatrix;
			lptr->m_Position = pos;
			lptr->m_Direction = dir;
			math::mat4 mat;
			tr.getOpenGLMatrix(&mat[0][0]);
			lptr->m_PositionMatrix = glm::scale(mat, math::vec3(infl));

			break;
		}
		case Light::LightType::Point:{
			auto lptr = RInput->m_PointLights.Allocate();
			if (!lptr) {
				AddLog(Warning, "Cannot allocate PointLight!");
				continue;
			}
			light = &lptr->m_Base;
			lptr->m_Attenuation = item.m_Attenuation;

			auto pos = convert(tr.getOrigin());

			lptr->m_Position = pos;
			tr.getOpenGLMatrix(&lptr->m_PositionMatrix[0][0]);

			break;
		}
		case Light::LightType::Directional:	{
			auto lptr = RInput->m_DirectionalLights.Allocate();
			if (!lptr) {
				AddLog(Warning, "Cannot allocate DirectionalLight!");
				continue;
			}
			light = &lptr->m_Base;
			break;
		}
		default:
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//entry is not valid and shall be removed
			continue;
		}

		*light = item.m_Base;
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
	entry.m_Attenuation = le.m_Attenuation;
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
