/*
  * Generated by cppsrc.sh
  * On 2016-08-10 17:23:57,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "ComponentManager.h"
#include "AbstractComponent.h"
#include "MeshComponent.h"
#include "TransformComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

RegisterApiNonClass(MeshComponent, &MeshComponent::RegisterScriptApi);
RegisterComponentID<MeshComponent> MeshComponent("Mesh");

MeshComponent::MeshComponent(ComponentManager * Owner) 
	: AbstractComponent(Owner)
{
}

MeshComponent::~MeshComponent() {
}

//------------------------------------------------------------------------------------------

void MeshComponent::RegisterScriptApi(ApiInitializer & root) {
	root
	.beginClass<MeshEntry>("cMeshEntry")
		.addProperty("Visible", &MeshEntry::IsVisible, &MeshEntry::SetVisible)
	.endClass()
	;
}

bool MeshComponent::PushEntryToLua(Handle h, lua_State *lua, int &luarets) {
	auto entry = GetEntry(h);
	if (!entry) {
		return true;
	}

	luarets = 1;
	luabridge::Stack<MeshEntry*>::push(lua, entry);

	return true;
}

//------------------------------------------------------------------------------------------

bool MeshComponent::Initialize() {
//	m_Array.MemZeroAndClear();
	m_Array.fill(MeshEntry());
	return true;
}

bool MeshComponent::Finalize() {
	return true;
}

void MeshComponent::Step(const MoveConfig &conf) {
	auto *EntityManager = GetManager()->GetWorld()->GetEntityManager();
	auto *HandleTable = GetManager()->GetWorld()->GetHandleTable();
	auto *tc = GetManager()->GetTransformComponent();

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

		if (!HandleTable->IsValid(this, item.m_Handle)) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further checks
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

		if (!item.m_Flags.m_Map.m_Visible) {
			continue;
		}

		conf.RenderList.push_back(std::make_pair(tcentry->m_GlobalMatrix, item.m_Model));
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "TransformComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseElement(LastInvalidEntry);
	}
}

bool MeshComponent::Load(xml_node node, Entity Owner, Handle &hout) {	
	auto name = node.child("Model").text().as_string(0);
	if (!name) {
		AddLogf(Error, "Attempt to load nameless Model!");
		return false;
	}
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();

	entry.m_Model = GetDataMgr()->GetModel(name);
	if (!entry.m_Model)
		return false;

	if (!entry.m_Model->Initialize()) {
		AddLogf(Error, "Failed to initialize model!");
		return false;
	}

	Handle &ch = hout;
	if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
		AddLogf(Error, "Failed to allocate handle!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	entry.m_Owner = Owner;
	entry.m_Handle = ch;

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Visible = node.child("Visible").text().as_bool(true);

	m_EntityMapper.SetHandle(entry.m_Owner, ch);

	return true;
}

void MeshComponent::ReleaseElement(size_t Index) {}

bool MeshComponent::GetInstanceHandle(Entity Owner, Handle &hout) {
	auto h = m_EntityMapper.GetHandle(Owner);
	if (!GetHandleTable()->IsValid(this, h)) {
		return false;
	}
	hout = h;
	return true;
}

MeshComponent::MeshEntry *MeshComponent::GetEntry(Handle h) {
	auto *ht = GetManager()->GetWorld()->GetHandleTable();
	HandleIndex hi;
	if (!ht->GetHandleIndex(this, h, hi)) {
		//AddLog(Debug, "Attempt to get MeshEntry for invalid Entity!");
		return nullptr;
	}
	return &m_Array[hi];
}

MeshComponent::MeshEntry *MeshComponent::GetEntry(Entity e) {
	return GetEntry(m_EntityMapper.GetHandle(e));
}

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 
