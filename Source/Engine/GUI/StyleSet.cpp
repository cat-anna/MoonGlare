/*
  * Generated by cppsrc.sh
  * On 2015-03-08 10:04:54,22
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "GUI.h"
#include <MoonGlare.h>

namespace MoonGlare {
namespace GUI {

SPACERTTI_IMPLEMENT_STATIC_CLASS(StyleSet);

StyleSet::StyleSet(StyleManager *Owner):
		BaseClass(),
		Regular(this, "Regular"),
		Focused(this, "Focused"),
		Hovered(this, "Hovered"),
		m_Owner(Owner),
		m_Name("Default") {

}

StyleSet::~StyleSet() {
}

//----------------------------------------------------------------

SharedAnimation StyleSet::GetAnimation(const Style *style, const string& Name) const {
	char buffer[256];
	sprintf(buffer, "%s.%s", style->GetName().c_str(), Name.c_str());
	string Aname = buffer;

	auto it = m_AnimCache.find(Aname);
	if (it == m_AnimCache.end()) {
		auto animit = m_AnimTable.find(Aname);
		if (animit == m_AnimTable.end())
			animit = m_AnimTable.find(Name);

		if (animit == m_AnimTable.end()) {
			AddLog(Error, "Unable to find animation " << Aname);
			return SharedAnimation();
		}
		m_AnimCache[Aname] = &animit->second;
		it = m_AnimCache.find(Aname);
	}

	SharedAnimation aptr = it->second->Ptr.lock();
	if (!aptr) {
		aptr = LoadAnimation(it->second);
		if (!aptr) {
			AddLog(Error, "Unable to load animation " << Aname);
			return SharedAnimation();
		}
	}

	return aptr;
}

SharedAnimation StyleSet::LoadAnimation(const AnimationInfo *info) const {
	SharedAnimation sa = std::make_shared<Animation>();
	if (!sa->LoadMeta(info->Description)) {
		AddLog(Error, "An error has occur during loading animation!");
		return SharedAnimation();
	}
	info->Ptr = WeakAnimation(sa);
	return sa;
}

//----------------------------------------------------------------

bool StyleSet::LoadMeta(xml_node node) {
	if (!node) return false;

	auto cstrName = node.attribute(xmlAttr_Name).as_string(0);
	if (!cstrName) {
		AddLog(Error, "Unnamed style definition. Ignored.");
		return false;
	}
	m_Name = cstrName;

	if (!Regular.LoadMeta(node.child("Regular"))) {
		AddLog(Error, "Unable to load focused style for " << m_Name);
	}

	if (!Focused.LoadMeta(node.child("Focused"))) {
		AddLog(Error, "Unable to load focused style for " << m_Name);
	}

	if (!Hovered.LoadMeta(node.child("Hovered"))) {
		AddLog(Warning, "Unable to load hovered style for '" << m_Name << "'. Using copy of focused style!");
		Hovered.Clone(Focused); 
	}

	auto nodeAnimations = node.child("Animations");
	if (nodeAnimations) {
		XML::ForEachChild(nodeAnimations, "Item", [this](xml_node node)->int {
			auto cstrName = node.attribute(xmlAttr_Name).as_string(0);
			if (!cstrName) {
				AddLog(Error, "Unnamed animation definition. Ignored.");
				return 0;
			}
			string aname = cstrName;
			m_AnimTable[aname].Description = node;
			return 0;
		});
	}
	return true;
}

} //namespace GUI 
} //namespace MoonGlare 
