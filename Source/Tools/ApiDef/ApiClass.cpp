#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiClass::ApiClass(const std::string &Name, ApiBaseType *Parent):
		ApiNamespace(Name, Parent, stClass){
}

const ApiBaseType* ApiClass::FindType(const std::string &name, unsigned Flags) const{
	if(Compare(name, (Flags | ffCompareExactName)))
		return this;
	return ApiNamespace::FindType(name, Flags);
}

bool ApiClass::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const {
	bool skip = (Flags & ffSkipOwnName) != 0;
	if(!skip && !Compare(Path, Flags)) return false;
	Flags &= ~ffSkipOwnName;
	const char *p = Path.c_str();
	bool inside = false;
	bool all = *p == '*';
	if(!all){
		inside = (p = strchr(p, '.'));
		if(!p) p = Path.c_str();
		else ++p;
	}
	
	if(!inside && !skip)
		Result.push_back(this);
		
	if(inside || all || skip) {
		for(NamespaceVector::const_iterator it = m_List.begin(), jt = m_List.end(); it != jt; ++it){
			(*it)->Search(p, Result, Flags);
		}
		
		if(!(Flags & ffNoFollowInheritence))
			for(LegacyVector::const_iterator it = m_Legacy.begin(), jt = m_Legacy.end(); it != jt; ++it){
				const ApiBaseType *base = it->GetType(this);
				if(!base) continue;
				base->Search(p, Result, Flags | ffSkipOwnName);
			}
	}

	return true;
}

void ApiClass::LoadFromXML(const pugi::xml_node Node){
	for(xml_node i = Node.first_child(); i; i = i.next_sibling()){
		const char *type = i.name();
		string name = i.attribute("Name").as_string("");
		if(name.empty()) continue;
		ApiBaseType *child = 0;
		switch(NamespaceSubclasses.UnsafeGet(type)){
			case stNamespace:
				child = NewNamespace(name);
				break;
			case stVariable:
				child = NewVariable(name);
				break;
			case stClass:
				child = NewClass(name);
				break;
			case stFunction:
				child = NewFunction(name);
				break;
			case stDerive:
				m_Legacy.push_back(TypePointer(name));
				continue;				
			default:
				continue;
		}
		child->LoadFromXML(i);
	}
}

void ApiClass::SaveToXML(pugi::xml_node node) const{
	node.append_attribute("Name") = m_Name.c_str();
	
	for(LegacyVector::const_iterator it = m_Legacy.begin(), jt = m_Legacy.end(); it != jt; ++it){
		const TypePointer &p = *it;
		p.SaveToXML(node.append_child("Derive"));
	}
	
	for(NamespaceVector::const_iterator it = m_List.begin(), jt = m_List.end(); it != jt; ++it){
		const ApiBaseType *p = *it;
		xml_node child = node.append_child(NamespaceSubclasses.GetText(p->GetNodeType()));
		p->SaveToXML(child);
	}
	
	//LegacyVector  NamespaceVector
}

std::string ApiClass::ToString(unsigned Mode) const {
	switch (Mode) {
		case scmNameWithPath:
		case scmLine:{
			stringstream ss;
			ss << "class " << ToString(Mode == scmLine ? scmName : scmPath) << ";";
			return ss.str();
		}
			
		case scmBegining:{
			stringstream ss;
			ss << "class " << m_Name;
			
			if(m_Legacy.size() > 0){
				ss << " : public ";
				bool b = false;
				for(LegacyVector::const_iterator it = m_Legacy.begin(), jt = m_Legacy.end(); it != jt; ++it){
					ss << it->GetTypeName();
					if(b) ss << ", ";
					else b = true;
				}
				
			}
			ss << " {";
			return ss.str();
		}
		
		default:
			return ApiNamespace::ToString(Mode);
	}
}

