/*
 * cPath.cpp
 *
 *  Created on: 18-10-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace DataClasses {
namespace Paths {

GABI_IMPLEMENT_ABSTRACT_CLASS(iPath);
RegisterApiDerivedClass(iPath, &iPath::RegisterScriptApi);

iPath::iPath():
		BaseClass(),
		m_Flags(0),
		m_Owner(0) {
}

iPath::~iPath() {
}

void iPath::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("iPath")
	.endClass();
}

bool iPath::LoadMeta(const xml_node Node) {
	return ReadNameFromXML(Node);
}

void iPath::SetOwner(PathRegister *Owner) {
	if (m_Owner){
		AddLogf(Debug, "Doing reset %s owner!", GetName().c_str());
	}
	if (!Owner){
		AddLogf(Debug, "Setting %s owner to null!", GetName().c_str());
	}
	m_Owner = Owner;
}

} // namespace Paths 
} // namespace DataClasses 
