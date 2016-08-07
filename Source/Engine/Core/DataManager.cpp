#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iSoundEngine.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/DataManager.h>
#include <Engine/DataClasses/iFont.h>
#include <Engine/Core/Scripts/GlobalContext.h>

#include <StarVFS/core/nStarVFS.h>

namespace MoonGlare {
namespace Core {
namespace Data {
#if 0
namespace InternalFS {

using Node = FileSystem::InternalContainer::FileNode;
using Flags = FileSystem::InternalContainer::FileFlags;

#ifdef _BUILDING_ENGINE_
extern const Node Root_folder[];
#else
const Node Root_folder[] ={
	{ }
};
#endif

const Node RootNode = { Flags::Folder, nullptr, 0, Root_folder, "Internal", };

};
#endif

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Manager)
RegisterApiInstance(Manager, &Manager::Instance, "Data");
RegisterApiDerivedClass(Manager, &Manager::RegisterScriptApi);

Manager::Manager() : cRootClass(), m_Flags(0) {
	SetThisAsInstance();
	m_Modules.reserve(StaticSettings::DataManager::MaxLoadableModules);
	new DataClasses::Texture();
	m_StringTables = std::make_unique<DataClasses::StringTable>();

//	AddLog(FixMe, "DataManager is not threadsafe");
}

Manager::~Manager() {
	Finalize();

	m_StringTables.reset();

	DataClasses::Texture::DeleteInstance();
}

//------------------------------------------------------------------------------------------

class DataManagerDebugScritpApi {
public:
#ifdef DEBUG_SCRIPTAPI
	static void ClearStringTables() {
		GetDataMgr()->m_StringTables->Clear();
	}
	static void ClearResources(Manager *mgr) {
		AddLogf(Warning, "Clearing resources");
		mgr->m_Fonts.Lock()->clear();
		mgr->m_StringTables->Clear();
	}
#endif
};

void Manager::RegisterScriptApi(::ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cDataManager")
		.addFunction("GetString", &ThisClass::GetString)
#ifdef DEBUG_SCRIPTAPI
		.addFunction("NotifyResourcesChanged", &ThisClass::NotifyResourcesChanged)
		.addFunction("ClearStringTables", Utils::Template::InstancedStaticCall<ThisClass, void>::get<&DataManagerDebugScritpApi::ClearStringTables>())
		.addFunction("ClearResources", Utils::Template::InstancedStaticCall<ThisClass, void>::callee<&DataManagerDebugScritpApi::ClearResources>())
#endif
	.endClass()
	;
}

//------------------------------------------------------------------------------------------

bool Manager::Initialize() {
	ASSERT(!IsInitialized());
	OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Resources, "RES");
	
	//GetFileSystem()->RegisterInternalContainer(&InternalFS::RootNode, FileSystem::InternalContainerImportPriority::Primary);
	//GetFileSystem()->LoadRegisteredContainers();

	SetInitialized(true);

	for (auto &it : Settings->Modules.List) {
		AddLogf(Debug, "Trying to load module '%s'", it.c_str());
		
		if (!GetFileSystem()->LoadContainer(it)) {
			AddLogf(Error, "Unable to open module: '%s'", it.c_str());
		}
	}

#ifdef DEBUG_RESOURCEDUMP
	{
		std::ofstream file("logs/vfs.txt");
		GetFileSystem()->DumpStructure(file);
	}
#endif

	return true;
}

bool Manager::Finalize() {
	if (!IsInitialized()) return true;

	DumpResources();

	m_Fonts.clear();
	m_Maps.clear();
	m_Models.clear();

	m_Modules.clear();

	SetInitialized(false);
	return true;
}

//------------------------------------------------------------------------------------------

bool Manager::LoadModule(StarVFS::Containers::iContainer *Container) {
	ASSERT(Container);
	ASSERT(IsInitialized());

	m_Modules.emplace_back();
	auto &mod = m_Modules.back();

	mod.m_Container = Container;
	mod.m_ModuleName = "?";

	auto cfid = Container->FindFile("/" xmlstr_Module_xml);
	StarVFS::ByteTable data;
	if (!Container->GetFileData(cfid, data)) {
		AddLogf(Error, "Failed to read module meta-data from container '%s'", Container->GetContainerURI().c_str());
		return true;
	}

	pugi::xml_document doc;
	if (!doc.load_string(data.get())) {
		AddLogf(Error, "Failed to parse container meta-data xml '%s'", Container->GetContainerURI().c_str());
		return true;
	}

	auto rootnode = doc.document_element();
	m_Configuration.LoadUpdate(rootnode.child("Options"));

	mod.m_ModuleName = rootnode.child("ModuleName").text().as_string("?");

	AddLogf(Hint, "Loaded module '%s' from container '%s'", mod.m_ModuleName.c_str(), Container->GetContainerURI().c_str());

#if 0

	if (!modptr->IsValid()) {
		AddLogf(Error, "Unable to import module '%s'", modptr->GetContainer()->GetFileName().c_str());
		return false;
	}
	if (!modptr->Open()) {
		AddLogf(Error, "Unable to open module '%s'", modptr->GetContainer()->GetFileName().c_str());
		return false;
	}

	DataClasses::NameClassList list;

	auto ptr = modptr.get();

	m_Modules.emplace_back(std::move(modptr));
	//append configuration
	m_Config.LoadMeta(ptr->GetConfig());
	//append resources definitions
	//append resources definitions to external managers

	return true;
#endif
	return true;
}

//------------------------------------------------------------------------------------------

void Manager::LoadGlobalData() {
	Core::GetScriptEngine()->LoadAllScripts();
	GetSoundEngine()->ScanForSounds();

	//for (auto it = m_Modules.rbegin(), jt = m_Modules.rend(); it != jt; ++it) {
	//}

	NotifyResourcesChanged();
}

//------------------------------------------------------------------------------------------

DataClasses::FontPtr Manager::GetConsoleFont() {
	return GetFont(m_Configuration.m_ConsoleFont);
}

DataClasses::FontPtr Manager::GetDefaultFont() { 
	return GetFont(DEFAULT_FONT_NAME); 
}

//------------------------------------------------------------------------------------------

MapPtr Manager::GetMap(const string& Name) {
	auto it = m_Maps.find(Name);
	if (it != m_Maps.end()) {
		auto &ptr = it->second;
		if (ptr.IsLoaded()) {
			return ptr.Get();
		}
	}
	AddLogf(Debug, "Map '%s' not found or not loaded. Trying to load.", Name.c_str());
	
//	AddLog(TODO, "Move ownership of loaded map xml to map instance");
	FileSystem::XMLFile xml;
	if (!GetFileSystem()->OpenResourceXML(xml, Name, DataPath::Maps)) {
		AddLogf(Error, "Unable to open master resource xml for map '%s'", Name.c_str());
		return nullptr;
	}
	string Class = xml->document_element().attribute(xmlAttr_Class).as_string(ERROR_STR);
	bool Shared = xml->document_element().attribute(xmlAttr_Shared).as_bool(true);
	auto ptr = DataClasses::Maps::MapClassRegister::CreateShared(Class, Name);
	if (!ptr) {
		AddLogf(Error, "Unable to create map class '%s' for object '%s'", Class.c_str(), Name.c_str());
		return 0;
	}
	AddLogf(Debug, "Loading map '%s' of class '%s'", Name.c_str(), Class.c_str());

	m_Maps[Name].Set(Shared ? ptr : nullptr, Class);
	NotifyResourcesChanged();
	return ptr;
}

DataClasses::FontPtr Manager::GetFont(const string &Name) {
	auto fonts = m_Fonts.Lock();

	auto it = fonts->find(Name);
	FontResPtr *ptr = nullptr;
	if (it == fonts->end()) {
		ptr = &fonts[Name];
		ptr->SetValid(false);
	} else {
		ptr = &it->second;
		if (ptr->IsLoaded())
			return ptr->Get();

		if (!ptr->IsValid()) {
			AddLogf(Error, "Font '%s' is invalid.", Name.c_str());
			return nullptr;
		}
	}
	AddLogf(Debug, "Font '%s' not found. Trying to load.", Name.c_str());

//	AddLog(TODO, "Move ownership of loaded font xml to map instance");
	FileSystem::XMLFile xml;
	if (!GetFileSystem()->OpenResourceXML(xml, Name, DataPath::Fonts)) {
		AddLogf(Error, "Unable to open master resource xml for font '%s'", Name.c_str());
		fonts.unlock();
		return GetDefaultFont();
	}
	string Class = xml->document_element().attribute(xmlAttr_Class).as_string(ERROR_STR);
	auto font = DataClasses::Fonts::FontClassRegister::CreateShared(Class, Name);
	if (!font) {
		AddLogf(Error, "Unable to create font class '%s' for object '%s'", Class.c_str(), Name.c_str());
		fonts.unlock();
		return GetDefaultFont();
	}
	AddLogf(Debug, "Loading font '%s' of class '%s'", Name.c_str(), Class.c_str());
	if (!font->Initialize()) {
		AddLogf(Error, "Unable to initialize font '%s'", Name.c_str());
		fonts.unlock();
		return GetDefaultFont();
	}
	ptr->Set(font, Class);
	ptr->SetValid(true);
	NotifyResourcesChanged();
	return font;
}

DataClasses::ModelPtr Manager::GetModel(const string& Name) {
	auto models = m_Models.Lock();

	auto it = models->find(Name);
	ModelResPtr *ptr = nullptr;
	if (it == models->end()) {
		ptr = &models[Name];
		ptr->SetValid(false);
	} else {
		ptr = &it->second;
		if (ptr->IsLoaded())
			return ptr->Get();

		if (!ptr->IsValid()) {
			AddLogf(Error, "Model '%s' is invalid.", Name.c_str());
			return nullptr;
		}
	}
	AddLogf(Debug, "Model '%s' not found. Trying to load.", Name.c_str());

//	AddLog(TODO, "Move ownership of loaded model xml to map instance");
	FileSystem::XMLFile xml;
	if (!GetFileSystem()->OpenResourceXML(xml, Name, DataPath::Models)) {
		AddLogf(Error, "Unable to open master resource xml for model '%s'", Name.c_str());
		return nullptr;
	}
	string Class = xml->document_element().attribute(xmlAttr_Class).as_string(ERROR_STR);
	auto model = DataClasses::Models::ModelClassRegister::CreateShared(Class, Name);
	if (!model) {
		AddLogf(Error, "Unable to create model class '%s' for object '%s'", Class.c_str(), Name.c_str());
		return nullptr;
	}

	if (!model->LoadFromXML(xml->document_element())) {
		AddLogf(Error, "Unable to load model '%s' of class '%s'", Name.c_str(), Class.c_str());
		delete ptr;
		return nullptr;
	}

	ptr->Set(model, Class);
	ptr->SetValid(true);
	NotifyResourcesChanged();
	return model;
}

const string& Manager::GetString(const string &Id, const string& TableName) {
	return m_StringTables->GetString(Id, TableName);
}

Core::ciScene* Manager::LoadScene(const string& Name, const string& Class) const {
	Core::ciScene* ptr = Core::Scene::SceneClassRegister::CreateClass(Class);
	if (!ptr) {
		AddLogf(Error, "Unable to create scene class '%s' for object '%s'", Class.c_str(), Name.c_str());
		return 0;
	}
	AddLogf(Debug, "Loading scene '%s' of class '%s'", Name.c_str(), Class.c_str());

	FileSystem::XMLFile doc;
	if (!GetFileSystem()->OpenResourceXML(doc, Name, DataPath::Scenes)) {
		AddLog(Warning, "Unable to load xml for scene: " << Name);
	} else {
		if (!ptr->SetMetaData(doc)) {
			AddLogf(Error, "Unable to scene map '%s' of class '%s'", Name.c_str(), Class.c_str());
			delete ptr;
			return nullptr;
		}
	}
	return ptr;
}

//------------------------------------------------------------------------------------------

#if 0
template <class C>
void Manager::ImportResources(DataModule *module, bool(DataModule::*srcfun)(DataClasses::NameClassList&)const, std::unordered_map<string, C> &container) {
	DataClasses::NameClassList list;
	(module->*srcfun)(list);
	for (auto in : list) {
		auto it = container.find(in.Name);
		if (it != container.end()) {
			AddLogf(Debug, "Shadowing resource '%s' from module '%s' by resource from '%s'",
					in.Name.c_str(),
					it->second.GetOwner()->GetModuleName().c_str(),
					module->GetModuleName().c_str());
			
			container.erase(it);
		}

		container.emplace(std::piecewise_construct, 
						 std::tuple<string>(in.Name), 
						 std::tuple<DataClasses::DataModule*, DataClasses::NameClassPair&>(module, in));
	}
}
#endif

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifdef DEBUG_DUMP

struct Dumper {
	template<class T>
	static void DumpRes(const T &t, const char* Name, std::ostream &out) {
		out << Name << ":\n";
		for (auto &it : t) {
			char buf[128];
			sprintf(buf, "%40s [class %-20s][Loaded: %s][use count: %d]\n",
					it.first.c_str(), it.second.GetClass().c_str(), (it.second.IsLoaded() ? "T" : "F"), it.second.UseCount());
			out << buf;
		}
		out << "\n";
	}
	template<class T>
	static void DumpPredef(const T &t, const char* Name, std::ostream &out) {
		out << Name << ":\n";
		for (auto &it : t) {
			char buf[128];
			sprintf(buf, "%40s [class %-20s]\n",
					it.first.c_str(), it.second.Class.c_str() );
			out << buf;
		}
		out << "\n";
	}
};

void Manager::DumpAllResources(std::ostream &out) {
	Dumper::DumpRes<>(m_Maps, "Maps", out);
	Dumper::DumpRes<>(*m_Fonts.Lock(), "Fonts", out);
	Dumper::DumpRes<>(*m_Models.Lock(), "Models", out);
}

#endif

void Manager::DumpResources() {
#ifdef DEBUG_DUMP
	static std::mutex mutex;
	static unsigned RevisionIndex = 1;
	LOCK_MUTEX(mutex);
	static std::ofstream file (DEBUG_LOG_FOLDER "resources.txt");
	file << "Revision index: " << RevisionIndex << "\n\n";
	++RevisionIndex;
	GetDataMgr()->DumpAllResources(file);
	Core::GetScriptEngine()->DumpScripts(file);
	Core::GetScenesManager()->DumpAllDescriptors(file);
	Graphic::GetShaderMgr()->DumpShaders(file);
	GetSoundEngine()->DumpContent(file);
	GetModulesManager()->DumpModuleList(file);
	::Core::GetGlobalContext()->Dump(file);
	file << "\n\n--------------------------------------------------------------------------------------\n" << std::flush;
#endif
}

#ifdef DEBUG_DUMP
void Manager::NotifyResourcesChanged() {
	JobQueue::QueueJob([this]{ DumpResources(); });
} 
#endif

} // namespace Data
} // namespace Core
} // namespace MoonGlare
