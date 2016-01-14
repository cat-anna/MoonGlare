#include "ApiBaseType.h"

using namespace std;
using namespace pugi;

ApiDefinition::ApiDefinition(): 
		m_Headers(), m_BuiltInTypes(this, "BuiltIn"), m_Locals(this, "Locals") {
	
	m_BuiltInTypes.NewType("int");
	m_BuiltInTypes.NewType("float");
	m_BuiltInTypes.NewType("string");
	// m_BuiltInTypes.NewType("int");
}

ApiDefinition::~ApiDefinition(){

}

ApiHeader *ApiDefinition::NewHeader(const std::string &HeaderName){
	ApiHeader *hdr = new ApiHeader(this, HeaderName);
	m_Headers.push_back(hdr);
	return hdr;
}

const ApiBaseType* ApiDefinition::FindType(const std::string &Name, unsigned Flags) const{
	for(HeaderVector::const_iterator it = m_Headers.begin(), jt = m_Headers.end(); it != jt; ++it){
		const ApiBaseType *t = (*it)->FindType(Name, Flags);
		if(t) return t;
	}
	return m_BuiltInTypes.FindType(Name, Flags);
}

void ApiDefinition::Search(const std::string &Path, ConstNamespaceVector &Result, unsigned Flags) const{
	for(HeaderVector::const_iterator it = m_Headers.begin(), jt = m_Headers.end(); it != jt; ++it){
		(*it)->Search(Path, Result, Flags);
	}
	m_BuiltInTypes.Search(Path, Result, Flags);
	m_Locals.Search(Path, Result, Flags);
}	

void ApiDefinition::SetFlag(unsigned FlagSet, bool State) {
	if(State) m_Flags |= FlagSet;
	else m_Flags &= ~FlagSet;
	
	if(FlagSet & adfDisableBuiltInTypes)
		m_BuiltInTypes.SetFlag(ffDisableType, State);
}

void ApiDefinition::Load(const std::string &FileName){
	xml_document doc;
	doc.load_file(FileName.c_str());

	xml_node root = doc.document_element();
	assert(root);
	for(xml_node i = root.first_child(); i; i = i.next_sibling()){
		if(strcmp(i.name(), "Header")) continue;
		string hdr_name = i.attribute("Name").as_string();
		NewHeader(hdr_name)->LoadFromXML(i);
	}
}

void ApiDefinition::Save(const std::string &FileName) const{
}

const ApiHeader *ApiDefinition::GetHeader(unsigned index) const {
	return m_Headers[index];
}

unsigned ApiDefinition::GetHeaderCount() const{
	return m_Headers.size();
}

int ApiDefinition::GetHeaderIndex(const std::string &Name) const {
	int i = 0;
	for(HeaderVector::const_iterator it = m_Headers.begin(), jt = m_Headers.end(); it != jt; ++it){
		if((*it)->Compare(Name, ffCompareExactName))
			return i;
		++i;
	}
	return -1;
}

void ApiDefinition::SetHeaderFlags(const std::string &HeaderName, unsigned FlagSet, bool State){
	int idx = GetHeaderIndex(HeaderName);
	if(idx < 0) return;
	m_Headers[idx]->SetFlag(FlagSet, State);
}

unsigned ApiDefinition::BuiltInCount() const {
	return m_BuiltInTypes.GetCount();
}

const ApiBaseType *ApiDefinition::GetBuiltIn(unsigned index) const {
	return m_BuiltInTypes.GetItem(index);
}	

void ApiDefinition::ClearLocals() {
	m_Locals.ClearMembers();
}

void ApiDefinition::AddLocal(const std::string &Name, const std::string &Type) {
	ConstNamespaceVector vec;
	m_Locals.Search(Name, vec);
	if(vec.size() > 0){
		ApiBaseType *t = const_cast<ApiBaseType*>(vec[0]);
		ApiVariable *v = dynamic_cast<ApiVariable*>(t);
		if(!v) return;
		v->SetType(Type);
		return;
	}
	m_Locals.NewVariable(Name)->SetType(Type);	
}

unsigned ApiDefinition::LocalsCount() const {
	return m_Locals.GetCount();
}

const ApiBaseType *ApiDefinition::GetLocal(unsigned index) const {
	return m_Locals.GetItem(index);
}

