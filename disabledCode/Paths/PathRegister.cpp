/** cPathsManager.cpp
 *
 *  Created on: 18-10-2013
 *      Author: Paweu
 */
#include <pch.h>
#if 0
#include <MoonGlare.h>

namespace MoonGlare {
namespace DataClasses {
namespace Paths {

SPACERTTI_IMPLEMENT_STATIC_CLASS(PathRegister)

PathRegister::PathRegister(iMap *Owner):
		BaseClass(),
		m_Owner(Owner) {
	THROW_ASSERT(Owner, "Path register requires non null owner!");
	m_PathMetaDocument.reset(0);
}

PathRegister::~PathRegister() {
	Finalize();
}

bool PathRegister::Initialize() {
	m_PathMap.clear();
	m_PathMetaDocument.reset();

	auto reader = GetOwner()->GetDataReader();

	if (!reader.OpenXML(m_PathMetaDocument, "Paths.xml")) {
		AddLogf(Warning, "Unable to open paths xml file for map '%s'", GetOwner()->GetName().c_str());
		return false;
	}

	XML::ForEachChild(m_PathMetaDocument->document_element(), "Item", [this](xml_node node) -> int {
		PathEntry entry;
		if (!entry.Info.LoadMeta(node)) {
			AddLog(Warning, "An error has occur during reading NameClass pair for path entry!");
			return 0;
		}

		entry.MetaNode = node;

		m_PathMap[entry.Info.Name] = std::move(entry);
		return 0;
	});

	if (m_PathMap.empty()) return true;

	return true;
}

bool PathRegister::Finalize() {
	m_PathMap.clear();
	m_PathMetaDocument.reset();
	return true;
}

iPathSharedPtr PathRegister::GetPath(const string& Name) const {
	if (!m_PathMetaDocument) {
		AddLogf(Error, "Attempt to querry empty PathRegister [Map:%s  Path:%s]", GetOwner()->GetName().c_str(), Name.c_str());
		return nullptr;
	}

	auto it = m_PathMap.find(Name);
	if (it == m_PathMap.end()) {
		AddLogf(Error, "Path not found! [Map:%s  Path:%s]", GetOwner()->GetName().c_str(), Name.c_str());
		return nullptr;
	}

	auto &entry = it->second;
	auto shptr = entry.Ptr.lock();
	if (!shptr) {
#if 0
		auto ptr = ::Core::Interfaces::CreateResourceClass<iPath>(entry.Info.Class);
		if (!ptr) {
			AddLogf(Error, "Unable to create path object! [Map:%s  Path:%s  Class:%s]", GetOwner()->GetName().c_str(), Name.c_str(), entry.Info.Class.c_str());
			return nullptr;
		}
		ptr->SetOwner(const_cast<PathRegister*>(this));
		ptr->LoadMeta(entry.MetaNode);
		if (!ptr->Initialize()) {
			delete ptr;
			AddLogf(Error, "Unable to initialzie path object! [Map:%s  Path:%s  Class:%s]", GetOwner()->GetName().c_str(), Name.c_str(), entry.Info.Class.c_str());
			return nullptr;
		}
		shptr = iPathSharedPtr(ptr);
		entry.Ptr = iPathWeakPtr(shptr);
#endif // 0

	}
	return shptr;
}

} // namespace Paths 
} // namespace DataClasses 
} //namespace MoonGlare 
#endif