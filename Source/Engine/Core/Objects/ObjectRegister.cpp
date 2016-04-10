/*
 * cObjectList.cpp
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

GABI_IMPLEMENT_CLASS_NOCREATOR(ObjectRegister)
RegisterApiDerivedClass(ObjectRegister, &ObjectRegister::RegisterScriptApi);

ObjectRegister::ObjectRegister() {
	m_Objects.resize(Configuration::Storage::Static::ObjectBuffer);
}

ObjectRegister::~ObjectRegister() {
	Clear();
}

void ObjectRegister::RegisterScriptApi(ApiInitializer &api) {
//	api
//	.beginClass<ObjectList::ListTypeIterator>("cObjectSubListCIterator")
//		.addFunction("Next", &ObjectList::ListTypeIterator::Next)
//		.addFunction("Ended", &ObjectList::ListTypeIterator::Ended)
//		.addFunction("Index", &ObjectList::ListTypeIterator::Index)
//		.addFunction("Get", &ObjectList::ListTypeIterator::Get)
//	.endClass()
//	.beginClass<ObjectList>("cObjectSubList")
//		.addFunction("size", &ObjectList::isize)
//		.addFunction("GetIterator", &ObjectList::GetIterator)
//	.endClass();
}

void ObjectRegister::Clear() {
	m_Generations.Clear();
	m_Objects.clear();
	m_Objects.resize(m_Objects.capacity());
}

std::pair<Object*, Handle> ObjectRegister::Allocate() {
	auto h = m_Generations.Allocate();

	if (!m_Generations.IsHandleValid(h)) {
		AddLog(Warning, "No more space!");
		return std::make_pair(nullptr, Handle());
	}

	h.SetType(Configuration::Handle::Types::Object);
	auto uptr = std::make_unique<Object>();
	auto rawptr = uptr.get();
	uptr->SetSelfHandle(h);
	m_ActiveObjects.push_back(rawptr);
	m_Objects[h.GetIndex()].swap(uptr);

	return std::make_pair(rawptr, h);
}

Handle ObjectRegister::Insert(std::unique_ptr<Object> obj) {
	auto h = m_Generations.Allocate();

	if (!m_Generations.IsHandleValid(h)) {
		AddLog(Warning, "Invalid handle!");
		return Handle();
	}

	h.SetType(Configuration::Handle::Types::Object);

	m_ActiveObjects.push_back(obj.get());
	obj->SetSelfHandle(h);
	m_Objects[h.GetIndex()].swap(obj);
	obj.reset();
	return h;
}

Handle ObjectRegister::Insert(Object* obj) {
	auto h = m_Generations.Allocate();

	if (!m_Generations.IsHandleValid(h)) {
		AddLog(Warning, "Invalid handle!");
		return Handle();
	}

	m_ActiveObjects.push_back(obj);
	h.SetType(Configuration::Handle::Types::Object);
	obj->SetSelfHandle(h);
	m_Objects[h.GetIndex()].reset(obj);

	return h;
}

void ObjectRegister::Release(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	if (!m_Generations.IsHandleValid(h)) {
		AddLog(Warning, "Invalid handle!");
		return;
	}

	auto ptr = m_Objects[h.GetIndex()].release();
	ptr->SetSelfHandle(Handle());
	m_Generations.Free(h);

	for (auto it = m_ActiveObjects.begin(), jt = m_ActiveObjects.end(); it != jt; ++it) {
		if (*it != ptr)
			continue;

		std::swap(it, --jt);
		m_ActiveObjects.pop_back();
		break;
	}
}

void ObjectRegister::Remove(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	if (!m_Generations.IsHandleValid(h)) {
		AddLog(Warning, "Invalid handle!");
		return;
	}

	auto ptr = std::move(m_Objects[h.GetIndex()]);
	m_Generations.Free(h);

	for (auto it = m_ActiveObjects.begin(), jt = m_ActiveObjects.end(); it != jt; ++it) {
		if (*it != ptr.get())
			continue;

		std::swap(it, --jt);
		m_ActiveObjects.pop_back();
		break;
	}

	ptr->Finalize();
	ptr.reset();
}

Object *ObjectRegister::Get(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	if (!m_Generations.IsHandleValid(h)) {
		AddLog(Warning, "Invalid handle!");
		return nullptr;
	}
	return m_Objects[h.GetIndex()].get();
}

bool ObjectRegister::LoadObjects(const xml_node SrcNode, GameScene *OwnerScene) {
	if (!SrcNode) return true;

	for (xml_node itnode = SrcNode.child("Item"); itnode; itnode = itnode.next_sibling("Item")) {
		const char* name = itnode.attribute(xmlAttr_Object).as_string(0);
		if (!name) {
			AddLog(Error, "An object definition without predef object name!");
			continue;
		}

		Object *obj = GetDataMgr()->LoadObject(name, OwnerScene);
		if (!obj) {
			AddLogf(Warning, "Unable to create object of name '%s'", name);
			continue;
		}

		if(!obj->LoadDynamicState(itnode)) {
			AddLog(Error, "Unable to load dynamic state of object. Pattern: '" << name << "'");
			delete obj;
			continue;
		}
		Insert(obj);
	}
	return true;
}

} //namespace Objects
} //namespace Core
