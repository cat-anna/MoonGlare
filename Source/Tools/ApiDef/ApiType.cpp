#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiType::ApiType(const std::string &Name, ApiBaseType *Parent, eSubclassType NodeType): 
		ApiBaseType(Name, Parent, NodeType) {
}


ApiType::ApiType(const std::string &Name, ApiBaseType *Parent): 
		ApiBaseType(Name, Parent, stType) {
}

const sFlagStruct TypeFlags[] = {
	{"Const", nfConstant, false},
//	{"", 0, false},
	{0, 0, false},
};

void ApiType::SaveToXML(pugi::xml_node node) const {
	WriteFlagPack(node, m_Flags, TypeFlags);
	node.append_attribute("Type") = m_Name.c_str();
}

void ApiType::LoadFromXML(const pugi::xml_node node) {
	ReadFlagPack(node, m_Flags, TypeFlags);
	m_Name = node.attribute("Name").as_string("");
}

const ApiBaseType* ApiType::FindType(const std::string &name, unsigned Flags) const {
	if(Compare(name, Flags | ffCompareExactName))return this;
	return 0;
}

std::string ApiType::ToString(unsigned Mode) const {
	switch (Mode) {	
		case scmLine:
		case scmNameWithPath:{
			std::stringstream ss;
			if(m_Flags & nfConstant) ss << "const ";
			ss << ToString(Mode == scmLine ? scmType : scmPath) << ";";
			return ss.str();
		} 

		case scmType:
			return GetName();	
		case scmName:
			return "";
	default:
		return ApiBaseType::ToString(Mode);
	}
}

std::string ApiType::CurrentPath() const{
	if(!m_Parent)
		return GetName();
	return m_Parent->CurrentPath() + '.' + GetName();
}