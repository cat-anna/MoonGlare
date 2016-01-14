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
}

ObjectRegister::~ObjectRegister() {
	Clear();
}

void ObjectRegister::RegisterScriptApi(ApiInitializer &api) {
	api
	.beginClass<ObjectList::ListTypeIterator>("cObjectSubListCIterator")
		.addFunction("Next", &ObjectList::ListTypeIterator::Next)
		.addFunction("Ended", &ObjectList::ListTypeIterator::Ended)
		.addFunction("Index", &ObjectList::ListTypeIterator::Index)
		.addFunction("Get", &ObjectList::ListTypeIterator::Get)
	.endClass()
	.beginClass<ObjectList>("cObjectSubList")
		.addFunction("size", &ObjectList::isize)
		.addFunction("GetIterator", &ObjectList::GetIterator)
	.endClass();
}

void ObjectRegister::Clear() {
	m_NameMap.clear();
	m_TypeMap.clear();
	for (auto *it : m_List)
		delete it;
	m_List.clear();
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
		Add(obj);
	}
	return true;
}

} //namespace Objects
} //namespace Core
