#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiFunction::ApiFunction(const std::string &Name, ApiBaseType *Parent):
		ApiBaseType(Name, Parent, stFunction), m_ResultType(){
}

const sFlagStruct FunctionFlags[] = {
	{"Const", nfConstant, false},
	{"MultiRet", nfMultiRet, false},
	{"MultiParam", nfMultiParam, false},
//	{"", 0},
	{0, 0, false},
};

/*
	if(!Compare(Path, Flags)) return false;
	const char *p = Path.c_str();
	bool inside = false;
	bool all = *p == '*';
	
	if(!all){
		inside = (p = strchr(p, '.'));
		if(!p) p = Path.c_str();
		else ++p;
	}
*/

bool ApiFunction::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const {
	if(!Compare(Path, Flags)) return false;
	const char* p = strchr(Path.c_str(), '.');
	if(!p) p = Path.c_str();
	
	if(!p || *p != '.'){
		Result.push_back(this);
		return true;
	}
	const ApiBaseType *t = m_ResultType.GetType(this);
	if(!t) return false;
	return t->Search(p, Result, Flags | ffSkipOwnName);
}

void ApiFunction::LoadFromXML(const pugi::xml_node Node){
	m_ResultType.SetType(Node.attribute("Return").as_string());
	m_Name = Node.attribute("Name").as_string();
	ReadFlagPack(Node, m_Flags, FunctionFlags);
	for(xml_node i = Node.first_child(); i; i = i.next_sibling()){
		sFunctionParameter p;
		p.Type = i.attribute("Type").as_string();
		p.Name = i.attribute("Name").as_string();
		m_Params.push_back(p);
	}
}

void ApiFunction::SaveToXML(pugi::xml_node Node) const{
	Node.append_attribute("Return") = m_ResultType.GetTypeName().c_str();
	Node.append_attribute("Name") = m_Name.c_str();
	WriteFlagPack(Node, m_Flags, FunctionFlags);
	int id = 0;
	for(ParamVector::const_iterator it = m_Params.begin(), jt = m_Params.end(); it != jt; ++it){
		const sFunctionParameter &p = *it;
		xml_node child = Node.append_child("Parameter");
		child.append_attribute("id") = id++;
		child.append_attribute("Type") = p.Type.GetTypeName().c_str();
		if(!p.Name.empty())
			child.append_attribute("Name") = p.Name.c_str();
	}	
}

unsigned ApiFunction::GetParamCount() const {
	return m_Params.size();
}

const sFunctionParameter& ApiFunction::GetParameter(unsigned index) const{
	return m_Params[index];
}

std::string ApiFunction::ToString(unsigned Mode) const {
	switch (Mode) {
		case scmNameWithPath: 
		case scmLine: {
			stringstream ss;
			if(m_Flags & nfMultiRet) ss << "...";
			else ss << m_ResultType.ToString(scmType);
			ss << " " << ToString(Mode == scmLine ? scmName : scmPath) << "(";
			if(m_Flags & nfMultiParam) ss << "...";
			else			
			for(ParamVector::const_iterator it = m_Params.begin(), beg = m_Params.begin(), jt = m_Params.end(); it != jt; ++it){
				if(beg != it) ss << ", ";
				ss << it->Type.ToString(scmType);
			}				
			ss << ");";
			return ss.str();
		}
		case scmType:
			return m_ResultType.ToString(scmType);
	default:
		return ApiBaseType::ToString(Mode);
	}
}
