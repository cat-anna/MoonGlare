#include <pch.h>
#include "ApiDefAutoGen.h"
#include <iostream>

namespace ApiDefAutoGen {

void WriteParams(xml_node node, const StringVector *Names, int paramid){
	node.append_attribute("ParamCount") = paramid;
}

//------------------------------------------------------------------------

bool Namespace::m_initialized = false;
const char* Namespace::m_OutFile = 0;
const char* Namespace::m_xmlRoot = 0;
std::map<std::string, std::string> Namespace::m_KnownClasees;
pugi::xml_document Namespace::xml_doc;

std::string Namespace::TranslateName(const char* name){
	Namespace::StringMap::iterator it;
	if((it = Namespace::m_KnownClasees.find(name)) != Namespace::m_KnownClasees.end())
		return it->second;
		
	char buff[256] = {};
	char *i = buff;
	for(const char* j = name; *j; ++j)
		if(*j != '*'){
			*i = *j;
			++i;
		}

	if((it = Namespace::m_KnownClasees.find(buff)) != Namespace::m_KnownClasees.end())
		return it->second;
	return buff;
}

Namespace::~Namespace() {
	if (!m_parent) {
		xml_doc.save_file((m_OutFile?m_OutFile:"ApiDef_autogen.xml"));
	}
}

Namespace &Namespace::addCFunction(const char *name, int(*)(lua_State*)){
	xml_node f = XML_NamedChild(m_root, "Function", name);
	f.append_attribute("MultiParam") = true;
	f.append_attribute("MultiRet") = true;
	return *this;
}

Namespace Namespace::beginNamespace(const char* name) {
	return Namespace(XML_NamedChild(m_root, "Namespace", name), this);
}

Namespace& Namespace::endNamespace() {
	if (!m_parent) return *this;
	else return *m_parent;
}

Namespace Namespace::Begin(const char* name) {
	if(!name) name = m_xmlRoot;
	if(!name) name = "_Error_";
	if (!m_initialized) {
		xml_doc.append_child("ApiDef");
		m_initialized = true;
		
		m_KnownClasees["const char*"] = "string";
		m_KnownClasees["char const*"] = "string";
		m_KnownClasees["double"] = "float";
		m_KnownClasees["char*"] = "string";
	}
	xml_node node = xml_doc.document_element().find_child_by_attribute("Header", "Name", name);
	if(!node){
		node = xml_doc.document_element().append_child("Header");
		node.append_attribute("Name") = name;
	}
	return Namespace(node);
}

void Namespace::SetRoot(const char* name){
	m_xmlRoot = name;
}

void Namespace::Initialize(const char* fn){
	m_OutFile = fn;
}

void Namespace::Finalize(){
	if(!m_OutFile) return;
	xml_doc.save_file(m_OutFile);
	xml_doc.reset();
	m_initialized = false;
	m_OutFile = 0;
}

//----------------------------------------------------

Class::Class(Namespace *parent, xml_node root, const char *name){
	m_parent = parent;
	m_root = XML_NamedChild(root, "Class", name);
}

Namespace& Class::endClass() {
	if (!m_parent) return *this;
	else return *m_parent;
}

//-----------------------------------------------------------------------------------------------

pugi::xml_document* OutCallsPicker::xml_doc;
bool OutCallsPicker::m_initialized = false;
std::vector<MakeFunRoot*>* OutCallsPicker::m_List;

void OutCallsPicker::RegisterDefinition(MakeFunRoot *ptr){
	if(!m_initialized)
		Initialize();
	m_List->push_back(ptr);
}

void OutCallsPicker::Initialize(){
	m_List = new std::vector<MakeFunRoot*>();
	xml_doc = new pugi::xml_document();
	xml_doc->append_child("ApiDefOutCalls");
	m_initialized = true;
}

void OutCallsPicker::WriteOutCalls(const char* filename){
	if(!m_initialized)
		Initialize();
	for(std::vector<MakeFunRoot*>::iterator it = m_List->begin(), jt = m_List->end(); it != jt; ++it){
		(*it)->WriteOutCall();
	}
	xml_doc->save_file(filename);
}

}//namespace
