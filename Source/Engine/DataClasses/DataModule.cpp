#include <pch.h>
#include <MoonGlare.h>
#include <Engine/DataClasses/iFont.h>

namespace DataClasses {

//-------------------------------------------------------------------------------------------------

GABI_IMPLEMENT_CLASS_NOCREATOR(DataModule);
IMPLEMENT_SCRIPT_EVENT_VECTOR(ModuleScriptEvents);
RegisterApiDerivedClass(DataModule, &DataModule::RegisterScriptApi);

DataModule::DataModule() :
		cRootClass(),
		m_Container(),
		m_ModuleName("?"),
		m_Flags(0),
		m_OwnedResources(),
		m_ScriptHandlers() 
{
}

DataModule::DataModule(FileSystem::iContainer *container) : DataModule() {
	m_Container = container;
}

DataModule::~DataModule() {
}

void DataModule::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<DataModule, cRootClass>("DataModule")
		//.addFunction("GetTexture", &DataModule::GetTexture_char)
		//.addFunction("GetPatternObject", &DataModule::GetPatternObject_char)
	.endClass();
}

//-------------------------------------------------------------------------------------------------

bool DataModule::IsValid() const {
	return m_Container != 0;
}

const xml_node DataModule::GetModuleDocNode() { 
	if (!m_ModuleDoc)
		return xml_node();
	return m_ModuleDoc->child(xmlstr_DataModule); 
}

const xml_node DataModule::GetModuleDocNode(const char* Name) const {
	if (!m_ModuleDoc)
		return xml_node();
	auto n = m_ModuleDoc->child(xmlstr_DataModule); 
	if (Name)
		n = n.child(Name);
	return n;
}

//-------------------------------------------------------------------------------------------------

bool DataModule::Open() {
	if (!IsValid()) {
		AddLog(Error, "Attempt to open invalid module!");
		return false;
	}
	if (IsOpened()) {
		AddLog(Warning, "Attempt to open already opened module!");
		return true;
	}

	if (!LoadPrivateXML(m_ModuleDoc, xmlstr_Module_xml, DataPath::Root)) {
		AddLog(Error, "Unable to load module metadata file!");
		return false;
	}

	xml_node DataNode = GetModuleDocNode();
	if (!DataNode) 
		return false;

	xml_node OptionsNode = DataNode.child(xmlModuleNode_Options);
	if (OptionsNode) {
		m_ModuleName = OptionsNode.child(xmlstr_ModuleName).text().as_string("");
		m_ScriptHandlers->LoadFromXML(OptionsNode);
	}

	xml_node node;

	//if (DataNode.child(xmlModuleNode_Player))					m_OwnedResources |= RT_Player;
	//if (DataNode.child(xmlModuleNode_GameEngine))	m_OwnedResources |= RT_GameEngine;

	SetOpened(true);
	return true;
}

//-------------------------------------------------------------------------------------------------

//no script handlers

//-------------------------------------------------------------------------------------------------

bool DataModule::LoadGameEgnine() {
#if 0
	if (!(m_OwnedResources & RT_GameEngine))
		ErrorLogAndReturn(false, "There is no GameEngine to load!");

	xml_document ModuleDoc;
	GetXML(xmlstr_Module_xml, ModuleDoc, dspRoot);
	xml_node DataNode = ModuleDoc.child(xmlstr_DataModule);

	xml_document GameEngineDoc;
	if (!DoOpenSubXML(DataNode.child(xmlModuleNode_GameEngine), GameEngineDoc))
		ErrorLogAndReturn(false, "Unable to open GameEngine XML file!");
	xml_node GameEngineNode = GameEngineDoc.document_element();
	string Class = GameEngineNode.attribute(xmlAttr_Class).as_string(cGameEngine::GetStaticTypeInfo()->Name);
	cGameEngine *ge = Core::Interfaces::CreateCoreClass<cGameEngine>(Class);
	if (!ge)
		ErrorLogAndReturn(false, "Unable to create refereed GameEngine class instance!");
	ge->SetDataModule(this);
	if (ge->LoadFromXML(GameEngineNode))
		m_LoadedResources |= RT_GameEngine;
	else
		ErrorLog("Unable to GameEngine!");
	return ge->Initialize();
#endif // 0
	LOG_NOT_IMPLEMENTED();
	return false;
}

//-------------------------------------------------------------------------------------------------

bool DataModule::LoadPrivateXML(FileSystem::XMLFile &doc, const string& file, DataPath origin) const {
	auto fr = m_Container->GetFileReader(xmlstr_Module_xml);
	if (!fr) {
		//log
		return false;
	}

	doc = std::make_unique<xml_document>();
	doc->load_buffer(fr->GetFileData(), fr->Size());

	//TODO: add xml parse result test!

	return true;
}

} //namespace DataClasses
