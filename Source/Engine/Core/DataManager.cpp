#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iSoundEngine.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/DataManager.h>
#include <Engine/Core/Scene/ScenesManager.h>
#include <Engine/DataClasses/iFont.h>

#include <DataClasses/Models/SimpleModelImpl.h>

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

Manager::Manager() : cRootClass() {
	SetThisAsInstance();

	m_ScriptEngine = nullptr;

	OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Resources, "RES");

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

bool Manager::Initialize(Scripts::ScriptEngine *ScriptEngine) {
	m_ScriptEngine = ScriptEngine;
	MoonGlareAssert(m_ScriptEngine);
	
	//GetFileSystem()->RegisterInternalContainer(&InternalFS::RootNode, FileSystem::InternalContainerImportPriority::Primary);
	//GetFileSystem()->LoadRegisteredContainers();

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
	m_Fonts.clear();

	for (auto &item : *m_Models.Lock()) {
		item.second->Finalize();
		item.second.reset();
	}
	m_Models.clear();

	m_Modules.clear();

	return true;
}

//------------------------------------------------------------------------------------------

bool Manager::LoadModule(StarVFS::Containers::iContainer *Container) {
	MoonGlareAssert(Container);
	MoonGlareAssert(m_ScriptEngine);

	m_Modules.emplace_back();
	auto &mod = m_Modules.back();

	mod.m_Container = Container;
	mod.m_ModuleName = "?";

	auto cfid = Container->FindFile("/" xmlstr_Module_xml);
	StarVFS::ByteTable data;
	if (!Container->GetFileData(cfid, data)) {
		AddLogf(Error, "Failed to read module meta-data from container '%s'", Container->GetContainerURI().c_str());
		return false;
	}

	pugi::xml_document doc;
	if (!doc.load_string((char*)data.get())) {
		AddLogf(Error, "Failed to parse container meta-data xml '%s'", Container->GetContainerURI().c_str());
		return false;
	}

	auto rootnode = doc.document_element();
	m_Configuration.LoadUpdate(rootnode.child("Options"));

	mod.m_ModuleName = rootnode.child("ModuleName").text().as_string("?");

	AddLogf(Hint, "Loaded module '%s' from container '%s'", mod.m_ModuleName.c_str(), Container->GetContainerURI().c_str());

	if (!LoadModuleScripts(Container)) {
		AddLogf(Error, "Failed to load module scripts!");
		return false;
	}

	return true;
}

bool Manager::LoadModuleScripts(StarVFS::Containers::iContainer *Container) {
	MoonGlareAssert(Container);
	MoonGlareAssert(m_ScriptEngine);

	auto cid = Container->GetContainerID();

	auto func = [this, cid, Container](StarVFS::ConstCString fname, StarVFS::FileFlags fflags, StarVFS::FileID CFid, StarVFS::FileID ParentCFid)->bool {
		auto dot = strrchr(fname, '.');
		if (!dot)
			return true;
		if (stricmp(dot, ".lua") != 0)
			return true;

		auto furi = StarVFS::MakeContainerFilePathURI(cid, fname);
		StarVFS::ByteTable data;
		if (!Container->GetFileData(CFid, data)) {
			AddLogf(Error, "Failed to get file data! (%d:%s)", (int)CFid, furi.c_str());
			return true;
		}

		AddLogf(Debug, "Loading script: %s (%u bytes)", furi.c_str(), data.size());

		m_ScriptEngine->ExecuteCode((char*)data.get(), data.byte_size(), furi.c_str());

		return true;
	};

	if (!Container->EnumerateFiles(func)) {
		AddLogf(Error, "Container %s does not support file enumeration!", Container->GetContainerURI().c_str());
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------

void Manager::LoadGlobalData() {
	GetSoundEngine()->ScanForSounds();

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
	if (it != models->end()) {
		if (!it->second) {
			AddLogf(Error, "Model '%s' is invalid.", Name.c_str());
			return nullptr;
		}
		return it->second;
	}
	AddLogf(Debug, "Model '%s' not found. Trying to load.", Name.c_str());

	auto model = std::make_shared<DataClasses::Models::SimpleModelImpl>(Name);
	if (!model->Load(Name)) {
		AddLogf(Error, "Unable to load model '%s'", Name.c_str());
		return nullptr;
	}
	models[Name] = model;
	NotifyResourcesChanged();
	return model;
}

const string& Manager::GetString(const string &Id, const string& TableName) {
	return m_StringTables->GetString(Id, TableName);
}

//------------------------------------------------------------------------------------------

#ifdef DEBUG_DUMP

struct Dumper {
	template<class T>
	static void DumpSharedRes(const T &t, const char* Name, std::ostream &out) {
		out << Name << ":\n";
		for (auto &it : t) {
			char buf[128];
			sprintf(buf, "%40s [class %-20s][Loaded: %s][use count: %d]\n",
					it.first.c_str(), "-",(it.second ? "T" : "F"), it.second.use_count());
			out << buf;
		}
		out << "\n";
	}
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
	Dumper::DumpRes<>(*m_Fonts.Lock(), "Fonts", out);
	Dumper::DumpSharedRes<>(*m_Models.Lock(), "Models", out);
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

	auto world = Core::GetEngine()->GetWorld();

	GetDataMgr()->DumpAllResources(file);
	
	auto sm = world->GetScenesManager();
	if(sm)
		sm->DumpAllDescriptors(file);

	Graphic::GetShaderMgr()->DumpShaders(file);
	GetSoundEngine()->DumpContent(file);
	GetModulesManager()->DumpModuleList(file);

	//TODO: dump global context
	file << "\n\n--------------------------------------------------------------------------------------\n" << std::flush;
#endif
}

#ifdef DEBUG_DUMP
void Manager::NotifyResourcesChanged() {
	GetEngine()->PushSynchronizedAction([this]{ DumpResources(); });
} 
#endif

} // namespace Data
} // namespace Core
} // namespace MoonGlare
