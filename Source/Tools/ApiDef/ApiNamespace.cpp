#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiNamespace::ApiNamespace(const std::string &Name, ApiBaseType *Parent, eSubclassType NodeType):
		ApiBaseType(Name, Parent, NodeType), m_List() {
}

ApiNamespace::ApiNamespace(const std::string &Name, ApiBaseType *Parent):
		ApiBaseType(Name, Parent, stNamespace), m_List() {
}

ApiNamespace::~ApiNamespace(){
}

const ApiBaseType* ApiNamespace::FindType(const std::string &name, unsigned Flags) const{
	for(NamespaceVector::const_iterator it = m_List.begin(), jt = m_List.end(); it != jt; ++it){
		const ApiBaseType *t = (*it)->FindType(name, Flags);
		if(t)
			return t;
	}
	return 0;
}

bool ApiNamespace::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const {
	bool hdr = GetNodeType() == stHeader;
	bool skip = Flags & ffSkipOwnName;
	if(!skip && !hdr && !Compare(Path, Flags)) return false;
	Flags &= ~ffSkipOwnName;
	const char *p = Path.c_str();
	bool inside = false, all = *p == '*';
	
	if(!hdr && !all){
		inside = (p = strchr(p, '.'));
		if(!p) p = Path.c_str();
		else ++p;
	}
	
	if(!inside && !hdr)
		Result.push_back(this);
		
	if(inside || all || hdr)
		for(NamespaceVector::const_iterator it = m_List.begin(), jt = m_List.end(); it != jt; ++it)
			(*it)->Search(p, Result, Flags);
	return true;
}

void ApiNamespace::RemoveMember(unsigned index) {
	if(index >= m_List.size()) return;
	m_List.erase (m_List.begin() + index);
} 

void ApiNamespace::RemoveMemberByType(eSubclassType type){
	for(unsigned i = 0; i < m_List.size(); ++i){
		if(m_List[i]->GetNodeType() != type) continue;
		m_List.erase (m_List.begin() + i);
	}
}

void ApiNamespace::ClearMembers() {
	m_List.clear();
}

unsigned ApiNamespace::GetCount() const{
	return m_List.size();
}

const ApiBaseType *ApiNamespace::GetItem(unsigned index) const{
	return m_List[index];
}

ApiNamespace *ApiNamespace::NewNamespace(const std::string &Name){
	ApiNamespace *n = new ApiNamespace(Name, this, stNamespace);
	m_List.push_back(n);
	return n;
}

ApiClass *ApiNamespace::NewClass(const std::string &Name){
	ApiClass *n = new ApiClass(Name, this);
	m_List.push_back(n);
	return n;
}

ApiVariable *ApiNamespace::NewVariable(const std::string &Name){
	ApiVariable *n = new ApiVariable(Name, this);
	m_List.push_back(n);
	return n;
}

ApiFunction *ApiNamespace::NewFunction(const std::string &Name){
	ApiFunction *n = new ApiFunction(Name, this);
	m_List.push_back(n);
	return n;
}

ApiConstant *ApiNamespace::NewConstant(const std::string &Name){
	return NULL;
//	ApiConstant *n = new ApiConstant(Name, this);
//	m_List.push_back(n);
//	return n;
}

ApiType *ApiNamespace::NewType(const std::string &Name){
	ApiType *n = new ApiType(Name, this);
	m_List.push_back(n);
	return n;
}


void ApiNamespace::LoadFromXML(const pugi::xml_node Node){
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
			default:
				continue;
		}
		child->LoadFromXML(i);
	}
}

void ApiNamespace::SaveToXML(pugi::xml_node Node) const{

}

std::string ApiNamespace::ToString(unsigned Mode) const {
	switch (Mode) {
		case scmNameWithPath:
		case scmLine:{
			stringstream ss;
			ss << "namespace " << ToString(Mode == scmLine ? scmName : scmPath) << ";";
			return ss.str();
		}
			
		case scmBegining: {
			stringstream ss;
			ss << "namespace " << ToString(scmName) << " {";
			return ss.str();
		}
			
		case scmEnding:
			return "};";
		default:
			return ApiBaseType::ToString(Mode);
	}
}
