/*
 * cScriptEvents.cpp
 *
 *  Created on: 06-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Scripts {

GABI_IMPLEMENT_ABSTRACT_CLASS(iScriptEvents);

bool iScriptEvents::SaveToXML(xml_node Parent) const {
	xml_node ScriptNode;
	auto &V = GetEventList();
	for(auto it = V.begin(), jt = V.end(); it != jt; ++it) {
		const string *str = &(this->*(it->second));
		if(str->empty())continue;
		if(!ScriptNode) ScriptNode = Parent.append_child(xmlNode_ScriptEvents);
		ScriptNode.append_child(it->first.c_str()).append_attribute("Invoke") = str->c_str();
	}
	return true;
}

bool iScriptEvents::LoadFromXML(xml_node Parent) {
    xml_node ScriptNode = Parent.child(xmlNode_ScriptEvents);
    if(!ScriptNode)return true;
	EventVector &V = GetEventList();
	for(auto it = V.begin(), jt = V.end(); it != jt; ++it) {
		string *str = &(this->*(it->second));
		(*str) = ScriptNode.child(it->first.c_str()).attribute("Invoke").as_string("");
	}
	return true;
}

void iScriptEvents::Set(const string& Event, const string& Function) {
	auto &ev = GetEventList();
	auto it = ev.find(Event);
	if (it == ev.end()) {
		AddLogf(Warning, "Event '%s' not found in class '%s'! Unable to set to function '%s'", 
				Event.c_str(), GetDynamicTypeInfo()->GetName(), Function.c_str());
		return;
	} 
	string &str = (this->*(it->second));
	str = Function;
}

void iScriptEvents::Assign(const iScriptEvents& Source) {
	if(&GetEventList() != &Source.GetEventList()){
		//throw
		AddLog(Error, "Cannot assign from different script events type!");
		return;
	}

	auto &V = GetEventList();
	for(auto it = V.begin(), jt = V.end(); it != jt; ++it) {
		(this->*(it->second)) = (Source.*(it->second));
	}
}

void iScriptEvents::InternalInfo(std::ostringstream &buff) const {
	EventVector &V = GetEventList();
	bool f = false;
	if(buff.tellp() > 0) buff << (f = true, " ");
	for(auto it = V.begin(), jt = V.end(); it != jt; ++it){
		const string &ptr = (this->*(it->second));
		if(f) buff << (f = false, " ");
		buff << it->first << "='" << ptr << "'";
	}
}

//----------------------------------------------------------------

iScriptEvents::EventVector& iScriptEvents::EventVector::Remove(const char* Name) {
	auto it = find(Name);
	if (it == end()) {
		AddLogf(Warning, "Attempt to remove event '%s' which does not exists!", Name);
		return *this;
	}
	erase(it);
	return *this;
}

} //namespace Scripts
} // namespace Core 
