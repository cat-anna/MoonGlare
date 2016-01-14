/*
 * ApiOutCalls.cpp
 *
 *  Created on: 28-10-2013
 *      Author: Paweu
 */

#include "ApiOutCalls.h"

using namespace pugi;

ApiOutCalls::ApiOutCalls(ApiDefinition *ApiDef): m_ApiDef(ApiDef) {
}

ApiOutCalls::~ApiOutCalls() {
	Clear();
}

void ApiOutCalls::Clear() {
	for(FunList::iterator it = m_FunList.begin(), jt = m_FunList.end(); it != jt; ++it)
		delete *it;
	m_FunList.clear();
}

bool ApiOutCalls::LoadFromXML(const pugi::xml_node Node) {
	Clear();
	if (!Node)
		return false;
	for (xml_node it = Node.child("Function"); it; it = it.next_sibling("Function")) {
		ApiFunction *af = new ApiFunction("", 0);
		af->LoadFromXML(it);
		m_FunList.push_back(new sFunDef());
		sFunDef * fd = m_FunList.back();
		fd->Caller = it.attribute("Caller").as_string("");
		fd->FunDef = af;
	}
	return true;
}

bool ApiOutCalls::Load(const std::string& Name) {
	xml_document doc;
	doc.load_file(Name.c_str());
	return LoadFromXML(doc.child("ApiDefOutCalls"));
}

const ApiFunction* ApiOutCalls::GetFunction(const std::string& FunName, const std::string& Caller) const {
	for(FunList::const_iterator it = m_FunList.begin(), jt = m_FunList.end(); it != jt; ++it)
		if((*it)->FunDef->Compare(FunName) && (Caller.empty() || !Caller.compare((*it)->Caller)))
			return (*it)->FunDef;
	return 0;
}
