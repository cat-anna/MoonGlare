#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiVariable::ApiVariable(const std::string &Name, ApiBaseType *Parent):
	ApiBaseType(Name, Parent, stVariable), m_Type(){
}

bool ApiVariable::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const {
	if(!Compare(Path, Flags)) return false;
	const char* p = strchr(Path.c_str(), '.');
	if(!p) p = Path.c_str();
	
	if(!p || *p != '.'){
		Result.push_back(this);
		return true;
	}
	const ApiBaseType *t = m_Type.GetType(this);
	if(!t) return false;
	return t->Search(p, Result, Flags | ffSkipOwnName);
}

const sFlagStruct VariableFlags[] = {
	{"Const", nfConstant, false},
//	{"", 0, 0},
	{0, 0, false},
};

void ApiVariable::LoadFromXML(const pugi::xml_node Node){
	m_Name = Node.attribute("Name").as_string("");
	m_Type.SetType(Node.attribute("Type").as_string(""));
	ReadFlagPack(Node, m_Flags, VariableFlags);
}

void ApiVariable::SaveToXML(pugi::xml_node Node) const{
	WriteFlagPack(Node, m_Flags, VariableFlags);
	Node.append_attribute("Name") = m_Name.c_str();
	Node.append_attribute("Type") = m_Type.GetTypeName().c_str();
}

std::string ApiVariable::ToString(unsigned Mode) const {
	switch (Mode) {	
		case scmLine:
		case scmNameWithPath:{
			std::stringstream ss;
			if(m_Flags & nfConstant) ss << "const ";
			ss << m_Type.ToString(scmType) << " " << ToString(Mode == scmLine ? scmName : scmPath) << ";";
			return ss.str();
		} 

		case scmType:
			return m_Type.ToString(scmType);		
	default:
		return ApiBaseType::ToString(Mode);
	}
}
