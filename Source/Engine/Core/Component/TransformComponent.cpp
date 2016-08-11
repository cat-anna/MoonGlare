/*
  * Generated by cppsrc.sh
  * On 2015-10-25 21:57:04,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "ComponentManager.h"
#include "ComponentRegister.h"
#include "AbstractComponent.h"
#include "TransformComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

RegisterApiNonClass(TransformComponent, &TransformComponent::RegisterScriptApi);
RegisterComponentID<TransformComponent> TransformComponentIDReg("Transform");

TransformComponent::TransformComponent(ComponentManager * Owner) 
		: AbstractComponent(Owner)
		, m_Allocated(0){
}

TransformComponent::~TransformComponent() {}

//---------------------------------------------------------------------------------------

void TransformComponent::RegisterScriptApi(ApiInitializer &root) {
	root
	.beginClass<TransformEntry>("cTransformComponentEntry")
		.addProperty("Position", &TransformEntry::GetPosition, &TransformEntry::SetPosition)
		.addProperty("Rotation", &TransformEntry::GetRotation, &TransformEntry::SetRotation)
		.addProperty("Scale", &TransformEntry::GetScale, &TransformEntry::SetScale)
	.endClass()
	;
}

bool TransformComponent::PushEntryToLua(Handle h, lua_State *lua, int &luarets) {
	auto entry = GetEntry(h);
	if (!entry) {
		return true;
	}

	luarets = 1;
	luabridge::Stack<TransformEntry*>::push(lua, entry);

	return true;
}

//------------------------------------------------------------------------------------------

bool TransformComponent::Initialize() {
	memset(&m_Array, 0, m_Array.size() * sizeof(m_Array[0]));

	HandleIndex index = m_Allocated++;
	auto &RootEntry = m_Array[index];
	RootEntry.m_Flags.ClearAll();
	RootEntry.m_Flags.m_Map.m_Valid = true;
	auto *EntityManager = GetManager()->GetWorld()->GetEntityManager();
	RootEntry.m_OwnerEntity = EntityManager->GetRootEntity();
	RootEntry.m_GlobalMatrix = math::mat4();
	RootEntry.m_Scale = Physics::vec3(1, 1, 1);

	auto *ht = GetManager()->GetWorld()->GetHandleTable();
	Handle h;
	if (!ht->Allocate(this, RootEntry.m_OwnerEntity, h, index)) {
		AddLog(Error, "Failed to allocate root handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	m_EntityMapper.SetHandle(RootEntry.m_OwnerEntity, h);

	return true;
}

bool TransformComponent::Finalize() {
	return true;
}

void TransformComponent::Step(const MoveConfig & conf) {
	auto *EntityManager = GetManager()->GetWorld()->GetEntityManager();
	auto *HandleTable = GetManager()->GetWorld()->GetHandleTable();

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 1; i < m_Allocated; ++i) {//ignore root entry
		auto &item = m_Array[i];

		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!HandleTable->IsValid(this, item.m_SelfHandle)) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further checks
			continue;
		}

		auto &gm = item.m_GlobalMatrix;
		auto &lm = item.m_LocalMatrix;
		item.m_LocalTransform.getOpenGLMatrix((float*)&lm);
//
	//	auto lm = *gm;
		lm[0] *= item.m_Scale[0];
		lm[1] *= item.m_Scale[1];
		lm[2] *= item.m_Scale[2];

		Entity ParentEntity;
		if (EntityManager->GetParent(item.m_OwnerEntity, ParentEntity)) {
			auto *ParentEntry = GetEntry(ParentEntity);
			gm = ParentEntry->m_GlobalMatrix * lm;
		//	item.m_GlobalScale = ParentEntry->m_GlobalScale * item.m_LocalScale;
		} else {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further processing
		}
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "TransformComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseElement(LastInvalidEntry);
	}
}

bool TransformComponent::Load(xml_node node, Entity Owner, Handle &hout) {
	Physics::vec3 pos;
	if (!XML::Vector::Read(node, "Position", pos)) {
		return false;
	}

	auto *ht = GetManager()->GetWorld()->GetHandleTable();
	Handle &h = hout;
	HandleIndex index = m_Allocated++;
	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();
	if (!ht->Allocate(this, Owner, h, index)) {
		AddLog(Error, "Failed to allocate handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	m_EntityMapper.SetHandle(Owner, h);

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_LocalTransform.setOrigin(pos);
	entry.m_LocalTransform.setRotation(Physics::Quaternion(0, 0, 0, 1));
	XML::Vector::Read(node, "Scale", entry.m_Scale, Physics::vec3(1, 1, 1));
	entry.m_SelfHandle = h;
	entry.m_OwnerEntity = Owner;

	return true;
}

TransformComponent::TransformEntry* TransformComponent::GetEntry(Handle h) {
	auto *ht = GetManager()->GetWorld()->GetHandleTable();
	HandleIndex hi;
	if (!ht->GetHandleIndex(this, h, hi)) {
		//AddLog(Debug, "Attempt to get TransformEntry for invalid Entity!");
		return nullptr;
	}
	return &m_Array[hi];
}

TransformComponent::TransformEntry* TransformComponent::GetEntry(Entity e) {
	return GetEntry(m_EntityMapper.GetHandle(e));
}

bool TransformComponent::GetInstanceHandle(Entity Owner, Handle &hout) {
	auto h = m_EntityMapper.GetHandle(Owner);
	if (!GetHandleTable()->IsValid(this, h)) {
		return false;
	}
	hout = h;
	return true;
}

void TransformComponent::ReleaseElement(size_t Index) {
	LOG_NOT_IMPLEMENTED();
}

//-------------------------------------------------------------------------------------------------

//void TransformComponent::BulletMotionStateProxy::getWorldTransform(btTransform & centerOfMassWorldTrans) const {
//	//	centerOfMassWorldTrans = m_Transform * m_CenterOfMass.inverse();
//}
//
//void TransformComponent::BulletMotionStateProxy::setWorldTransform(const btTransform & centerOfMassWorldTrans) {
//	//	m_Transform = centerOfMassWorldTrans * m_CenterOfMass;
//}

//-------------------------------------------------------------------------------------------------

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 
