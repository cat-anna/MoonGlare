/*
 * cObjectList.cpp
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>

#include <Core/Component/ComponentManager.h>
#include <Core/Component/AbstractComponent.h>	
#include <Core/Component/TransformComponent.h>
#include <Core/Component/ComponentRegister.h>

#include <Core/EntityBuilder.h>

namespace MoonGlare {
namespace Core {
namespace Objects {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(ObjectRegister)

ObjectRegister::ObjectRegister(World* world, GameScene *OwnerScene) :
		BaseClass(),
		WorldUser(world),
		m_OwnerScene(OwnerScene) {
	m_Memory = std::make_unique<Memory>();
	Clear();
}

ObjectRegister::~ObjectRegister() {
	for (auto& it : m_Memory->m_ObjectPtr)
		it.reset();
	m_Memory.reset();
}

void ObjectRegister::Clear() {
	for (auto& it: m_Memory->m_ObjectPtr) 
		it.reset();

	m_Memory->m_HandleAllocator.Rebuild();
//	m_Memory->m_GlobalMatrix.fill(math::mat4());
//	m_Memory->m_LocalMatrix.fill(math::mat4());
	Space::MemZero(m_Memory->m_Parent);

	Handle h;
	if (!m_Memory->m_HandleAllocator.Allocate(h)) {
		THROW_ASSERT(false, "critical error");
	}
	m_Memory->m_HandleAllocator.SetMapping(h, 0);
	m_Memory->m_ObjectPtr[0] = std::make_unique<Object>();
	m_Memory->m_ObjectPtr[0]->SetSelfHandle(h);
	m_Memory->m_Entity[0] = GetWorld()->GetEntityManager()->GetRootEntity();
}

Handle ObjectRegister::GetRootHandle() {
	auto h = m_Memory->m_HandleAllocator.GetHandleFromIndex(0);
	h.SetType(Configuration::Handle::Types::Object);
	return h;
}

Handle ObjectRegister::NewObject(Handle Parent) {
	return Insert(std::make_unique<Object>(), Parent);
}

Handle ObjectRegister::Insert(std::unique_ptr<Object> obj, Handle Parent) {
	ASSERT_HANDLE_TYPE(Object, Parent);

	if (!obj) {
		AddLog(Error, "Attempt to add null object!");
		return Handle();
	}

	if (!m_Memory->m_HandleAllocator.IsHandleValid(Parent)) {
		AddLog(Error, "Parent handle is not valid!");
		return Handle();
	}

	Handle h;
	if (!m_Memory->m_HandleAllocator.Allocate(h)) {
		THROW_ASSERT(false, "critical error");
	}
	if (!m_Memory->m_HandleAllocator.IsHandleValid(h)) {
		AddLog(Error, "No more space!");
		return Handle();
	}
	auto index = m_Memory->m_HandleAllocator.Allocated() - 1;
	m_Memory->m_HandleAllocator.SetMapping(h, index);

	h.SetType(Configuration::Handle::Types::Object);

	obj->SetSelfHandle(h);
	obj->SetOwnerRegister(this);
	m_Memory->m_ObjectPtr[index].swap(obj);
	m_Memory->m_Parent[index] = Parent;
	m_Memory->m_HandleIndex[index] = h.GetIndex();
	auto eparent = GetEntity(Parent);
	auto em = GetWorld()->GetEntityManager();
	Entity e;
	em->Allocate(eparent, e);

//	AddLog(Error, "Allocated " << e << " Parent:" << eparent);

	m_Memory->m_Entity[index] = e;

	return h;
}

void ObjectRegister::Release(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);

	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return;
	}

	m_Memory->m_ObjectPtr[idx]->SetSelfHandle(Handle());
	m_Memory->m_ObjectPtr[idx]->SetOwnerRegister(nullptr);
	m_Memory->m_ObjectPtr[idx].release();
	m_Memory->m_Parent[idx] = Handle();
	m_Memory->m_HandleAllocator.Free(h);
	GetWorld()->GetEntityManager()->Release(m_Memory->m_Entity[idx]);
	Reorder(idx);
}

void ObjectRegister::Remove(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);

	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return;
	}

	m_Memory->m_ObjectPtr[idx].reset();
	m_Memory->m_Parent[idx] = Handle();
	m_Memory->m_HandleAllocator.Free(h);
	GetWorld()->GetEntityManager()->Release(m_Memory->m_Entity[idx]);
	Reorder(idx);
}

void ObjectRegister::Reorder(size_t start) {
	throw "ObjectRegister - attempt to reorder!";
}

Handle ObjectRegister::GetParentHandle(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return Handle();
	}
	return m_Memory->m_Parent[idx];
}

Entity ObjectRegister::GetEntity(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return Entity();
	}
	return m_Memory->m_Entity[idx];
}

Entity ObjectRegister::GetParentEntity(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return Entity();
	}
	Entity Parent;
	GetWorld()->GetEntityManager()->GetParent(m_Memory->m_Entity[idx], Parent);
	return Parent;
}

Object *ObjectRegister::Get(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return nullptr;
	}
	return m_Memory->m_ObjectPtr[idx].get();
}

bool ObjectRegister::InitializeObjects() {
	bool ret = true;
	for (auto &it : *this)
		ret &= it->Initialize();
	return ret;
}

Handle ObjectRegister::LoadObject(Handle Parent, xml_node MetaXML, GameScene *OwnerScene) {
	auto objH = NewObject(Parent);
	Object *obj = Get(objH);
	obj->SetOwnerScene(OwnerScene);
	Entity objE = GetEntity(objH);

	if (!obj->LoadPattern(MetaXML)) {
		AddLogf(Error, "An Error has occur during loading meta of predef object (%s)", MetaXML.attribute("Name").as_string(ERROR_STR));
		Remove(objH);
		return Handle();
	}

	EntityBuilder eb(&OwnerScene->GetComponentManager());
	eb.LoadComponents(objE, MetaXML);

	for (xml_node it = MetaXML.child("Child"); it; it = it.next_sibling("Child")) {
		auto h = LoadObject(objH, it, OwnerScene);
		Object *childobj = Get(objH);
		if (!childobj) {
			AddLogf(Error, "Loading predef object child failed (%s)", it.attribute("Name").as_string(ERROR_STR));
			continue;
		}
	}

	return objH;
}

Handle ObjectRegister::LoadObject(const std::string &Name, GameScene *OwnerScene, Handle Parent) {
	FileSystem::XMLFile Meta;
	if (!GetFileSystem()->OpenResourceXML(Meta, Name, DataPath::Objects)) {
		AddLogf(Error, "Unable to open master resource xml for object '%s'", Name.c_str());
		return Handle();
	}

	if (!Get(Parent)) {
		Parent = GetRootHandle();
	}

	auto objH = LoadObject(Parent, Meta->document_element(), OwnerScene);
	Object *obj = Get(objH);
	if (!obj) {
		AddLogf(Error, "Loading predef object failed: '%s'", Name.c_str());
		Remove(objH);
		return Handle();
	}

	return objH;
}

bool ObjectRegister::LoadObjects(const xml_node SrcNode, GameScene *OwnerScene) {
	if (!SrcNode) 
		return true;

	auto RootHandle = GetRootHandle();

	for (xml_node itnode = SrcNode.child("Item"); itnode; itnode = itnode.next_sibling("Item")) {
		const char* name = itnode.attribute(xmlAttr_Object).as_string(0);
		if (!name) {
			AddLog(Error, "An object definition without predef object name!");
			continue;
		}

		auto objH = LoadObject(name, OwnerScene, RootHandle);
		Object *obj = Get(objH);
		if (!obj) {
			//already logged
			continue;
		}

		if (!obj->LoadDynamicState(itnode)) {
			AddLog(Error, "Unable to load dynamic state of object. Pattern: '" << name << "'");
			Remove(objH);
			continue;
		}
	}

	return true;
}

void ObjectRegister::Process(const MoveConfig &conf) {
	auto *cm = &m_OwnerScene->GetComponentManager();
	auto *tc = cm->GetTransformComponent();

	for (size_t i = 1, j = m_Memory->m_HandleAllocator.Allocated(); i < j; ++i) {

		size_t pid;
		if (!m_Memory->m_HandleAllocator.GetMapping(m_Memory->m_Parent[i], pid)) {
			continue;
		}

		auto obj = m_Memory->m_ObjectPtr[i].get();
		math::mat4 *gm = nullptr;
		auto *tcentry = tc->GetEntry(m_Memory->m_Entity[i]);

		if (!tcentry) {
			gm = &m_Memory->m_GlobalMatrix[i];
			auto &lm = m_Memory->m_LocalMatrix[i];
			obj->GetPositionTransform().getOpenGLMatrix((float*)&lm);
			*gm = m_Memory->m_GlobalMatrix[pid] * lm;
		} else {
			gm = &tcentry->m_GlobalMatrix;
			obj->SetPositionTransform(tcentry->m_LocalTransform);
		}
	}
}

Object *ObjectRegister::GetFirstObjectByName(const std::string &Name) {
	for (size_t i = 0; i < m_Memory->m_HandleAllocator.Allocated(); ++i) {
		auto &optr = m_Memory->m_ObjectPtr[i];
		if (!optr)
			continue;
		if (optr->GetName() == Name)
			return optr.get();
	}
	return nullptr;
}

} //namespace Objects
} //namespace Core
} //namespace MoonGlare 
