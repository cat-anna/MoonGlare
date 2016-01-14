#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

cNamespaceTypeHash::cNamespaceTypeHash(){
	Add("Namespace", stNamespace);
	Add("Variable", stVariable);
	Add("Property", stVariable);
	Add("Constant", stConstant);
	Add("Class", stClass);
	Add("Function", stFunction);
	Add("Derive", stDerive);
	Add("Header", stHeader);
	Add("Constructor", stConstructor);
}

cNamespaceTypeHash NamespaceSubclasses;

ApiBaseType::ApiBaseType(const std::string &Name, ApiBaseType *Parent, eSubclassType NodeType):
		m_Parent(Parent), m_Name(Name), m_NodeType(NodeType), m_Flags() {

	if(dynamic_cast<ApiClass*>(Parent))
    	m_Flags |= nfOwnerIsClass;
}

ApiBaseType::~ApiBaseType(){
}

bool ApiBaseType::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const {
	return false;
}

bool ApiBaseType::Compare(const std::string &Path, unsigned Flags) const {
	const char *m = m_Name.empty() ? 0 : m_Name.c_str();
	const char *p = Path.empty() ? 0 : Path.c_str();
	bool exact = Flags & ffCompareExactName;
	if(!p || m == p) return true;
	if(!m) return false;
	if(!exact){
		if(*p == '*') return true;
		if(*p == '.') ++p;
	}
	while(true){
		if(!*p) return !exact || !*m;
		if(!*m) return *p == '.';
		if((*m | 0x20) != (*p | 0x20)) return false;
		++m;
		++p;
	}
}

const ApiBaseType* ApiBaseType::FindType(const std::string &name, unsigned Flags) const{
	return 0;
}

void ApiBaseType::SetFlag(unsigned FlagSet, bool State) {
	if(State) m_Flags |= FlagSet;
	else m_Flags &= ~FlagSet;
}

void ApiBaseType::WriteFlagPack(pugi::xml_node Node, unsigned Flags, const sFlagStruct *FlagPack) const{
	for(const sFlagStruct *i = FlagPack; i->Name; ++i){
		bool f = Flags & i->Flag;
		if(f == i->Default) continue;
		Node.append_attribute(i->Name) = f;
	}
}

const ApiHeader* ApiBaseType::GetRootHeader() const  {
	if(!m_Parent) return 0;
	return m_Parent->GetRootHeader();
}

void ApiBaseType::ReadFlagPack(pugi::xml_node Node, unsigned &Flags, const sFlagStruct *FlagPack) const{
	for(const sFlagStruct *i = FlagPack; i->Name; ++i){
		bool f = Node.attribute(i->Name).as_bool(i->Default);
		if(f)
			Flags |= i->Flag;
		else 
			Flags &= ~i->Flag;
	}
}

std::string ApiBaseType::ToString(unsigned Mode) const {
	switch (Mode) {
		case scmName:
			return m_Name;	
		case scmPath:
			return CurrentPath();	
	default:
		return "";
	}
}

std::string ApiBaseType::CurrentPath() const{
	if(!m_Parent)
		return GetName();
	return m_Parent->CurrentPath() + '.' + GetName();
}
