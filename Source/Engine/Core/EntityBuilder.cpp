/*
  * Generated by cppsrc.sh
  * On 2016-08-14 22:04:32,35
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "Component/ComponentManager.h"
#include "Component/ComponentRegister.h"
#include "Component/AbstractComponent.h"
#include "EntityBuilder.h"

namespace MoonGlare {
namespace Core {

EntityBuilder::EntityBuilder(Component::ComponentManager *Manager)
		: m_Manager(Manager) {
}

EntityBuilder::~EntityBuilder() {
}

bool EntityBuilder::Build(Entity Owner, const char *PatternUri, Entity &eout) {
	XMLFile xml;
	if (!GetFileSystem()->OpenXML(xml, PatternUri)) {
		AddLogf(Error, "Failed to open uri: %s", PatternUri);
		return false;
	}
	
	unsigned count = BuildChild(Owner, xml->document_element(), eout);

	if (count == 0) {
		AddLogf(Error, "No elements has been loaded from: %s", PatternUri);
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------

unsigned EntityBuilder::BuildChild(Entity Owner, pugi::xml_node node, Entity &eout) {
	auto world = m_Manager->GetWorld();
	auto em = world->GetEntityManager();
	if (!em->IsValid(Owner)) {
		AddLogf(Error, "Attempt to build object with invalid owner entity!");
		return false;
	}

	Entity child;
	if (!em->Allocate(Owner, child)) {
		AddLogf(Error, "Failed to allocate entity!");
		return false;
	}

	auto c = ProcessXML(child, node);
	if (c == 0) {
		AddLogf(Error, "No elements has been loaded!");
		em->Release(child);
		return 0;
	}
	eout = child;
	return c;
}

unsigned EntityBuilder::ProcessXML(Entity Owner, pugi::xml_node node) {
	unsigned count = 0;
	for (auto it = node.first_child(); it; it = it.next_sibling()) {

		const char *nodename = it.name();
		auto hash = Space::Utils::MakeHash32(nodename);
		
		switch (hash) {
		case "Component"_Hash32: {
			Handle h;
			if (LoadComponent(Owner, it, h))
				++count;
			break;
		}
		case "Child"_Hash32: {
			Entity child;
			auto c = BuildChild(Owner, it, child);
			if (c == 0) {
				AddLogf(Error, "Failed to load child!");
				return 0;
			}
			count += c;
			break;
		}
		default:
			AddLogf(Warning, "Unknown node: %s", nodename);
			continue;
		}
	}
	return count;
}

unsigned EntityBuilder::LoadComponents(Entity Owner, pugi::xml_node node) {
	unsigned count = 0;
	for (auto it = node.child("Component"); it; it = it.next_sibling("Component")) {
		Handle h;
		if (LoadComponent(Owner, it, h))
			++count;
	}
	return count;
}

bool EntityBuilder::LoadComponent(Entity Owner, pugi::xml_node node, Handle & hout) {
	ComponentID cid = 0;
	{
		auto idxml = node.attribute("Id");
		if (idxml) {
			cid = idxml.as_uint(0);
		} else {
			auto namexml = node.attribute("Name");
			if (namexml && Component::ComponentRegister::GetComponentID(namexml.as_string(""), cid)) {
				//found
			} else {
				AddLog(Error, "Component definition without id or name!");
				return false;
			}
		}
	}

	if (cid == 0) {
		AddLogf(Warning, "Unknown component!", cid);
		return false;
	}

	auto c = m_Manager->GetComponent(cid);
	if (!c) {
		AddLogf(Warning, "No such component: %d", cid);
		return false;
	}

	if (!c->Load(node, Owner, hout)) {
		AddLog(Error, "Failure during loading component!");
		return false;
	}

	return true;
}

} //namespace Core 
} //namespace MoonGlare 

