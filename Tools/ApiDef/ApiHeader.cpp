#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiHeader::ApiHeader(ApiDefinition *Owner, const std::string &Name):
	ApiNamespace(Name, 0, stHeader), m_Owner(Owner){

}

ApiHeader::~ApiHeader(){

}

const ApiBaseType* ApiHeader::FindType(const std::string &Name, unsigned Flags) const {
	if(CheckFlag(ffDisableType)) return 0;
	return ApiNamespace::FindType(Name, Flags);
};

bool ApiHeader::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const {
	if(CheckFlag(ffDisableType)) return false;
	
	const char *p = Path.c_str();
	if(*p == '.') 
		++p;

	return ApiNamespace::Search(p, Result, Flags);
}


void ApiHeader::LoadFromXML(const pugi::xml_node node){
	ApiNamespace::LoadFromXML(node);
}

void ApiHeader::SaveToXML(pugi::xml_node node) const{
	ApiNamespace::SaveToXML(node);
}

const ApiHeader* ApiHeader::GetRootHeader() const  {
	return this;
}

std::string ApiHeader::ToString(unsigned Mode) const {
	char buff[1024] = {};
	switch (Mode) {
		case scmNameWithPath:
		case scmLine:
			sprintf(buff, "header %s;", m_Name.c_str());
			return buff;
			
		case scmBegining:
			sprintf(buff, "header %s {", m_Name.c_str());
			return buff;
	default:
		return ApiNamespace::ToString(Mode);
	}
}

