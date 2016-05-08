#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include "GlobalContext.h"

namespace Core {
namespace Scripts {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(cScriptEngine)
RegisterApiInstance(cScriptEngine, &cScriptEngine::Instance, "ScriptEngine");
RegisterApiDerivedClass(cScriptEngine, &cScriptEngine::RegisterScriptApi);

cScriptEngine::cScriptEngine() :
		cRootClass(),
		m_Flags(0) {
	SetThisAsInstance();

	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Script, "SCRI");
}

cScriptEngine::~cScriptEngine() {
	Finalize();
}

//---------------------------------------------------------------------------------------

void cScriptEngine::RegisterScriptApi(ApiInitializer &root) {
#ifdef DEBUG_SCRIPTAPI
	struct T {
		static void KillAllScripts() {
			GetEngine()->PushSynchronizedAction([](){
				GetScriptEngine()->KillAllScripts();
			});
		}
	};
#endif

	root
	.deriveClass<ThisClass, BaseClass>("cScriptEngine")
		.addFunction("LoadCode", &ThisClass::LoadCode)
		.addFunction("DefferExecution", &ThisClass::DefferExecution)
#ifdef DEBUG_SCRIPTAPI
		.addFunction("CollectGarbage", &ThisClass::CollectGarbage)
		.addFunction("PrintMemoryInfo", &ThisClass::PrintMemoryInfo)
		.addFunction("KillAllScripts", Utils::Template::InstancedStaticCall<ThisClass, void>::get<&T::KillAllScripts>())
#endif
	.endClass();
}

//---------------------------------------------------------------------------------------

void cScriptEngine::DefferExecution(string fname, int parameter) {
	JobQueue::QueueJob([fname, parameter]() {
		ScriptProxy::RunFunction<int>(fname.c_str(), parameter);
	});
}

//---------------------------------------------------------------------------------------

void cScriptEngine::CollectGarbage() {
	if (m_Script)
		m_Script->CollectGarbage();
}

void cScriptEngine::PrintMemoryInfo() {
	if (m_Script)
		m_Script->PrintMemoryUsage();
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::Initialize() {
	if (IsReady()) return true;

	new GlobalContext();
	GlobalContext::Instance()->Initialize();

	SetReady(ConstructScript());
	return IsReady();
}

bool cScriptEngine::Finalize() {
	if (!IsReady()) return true;
	SetReady(false);

	KillAllScripts();

	GlobalContext::Instance()->Finalize();
	GlobalContext::DeleteInstance();

	return true;
}

//---------------------------------------------------------------------------------------

void cScriptEngine::KillAllScripts() {
	LOCK_MUTEX(m_Mutex);

	while (!m_ScriptList.empty()) {
		auto fr = m_ScriptList.back().lock();
		m_ScriptList.pop_back();
		if (!fr)
			continue;

		fr->DropScript();
	}

	DestroyScript();
}

void cScriptEngine::LoadAllScriptsImpl() {
	FileSystem::FileInfoTable files;
	if (!GetFileSystem()->EnumerateFolder(DataPath::Scripts, files, true)) {
		AddLog(Error, "Unable to look for scripts!");
	}

	using FileInfo = MoonGlare::FileSystem::FileInfo;
	std::sort(files.begin(), files.end(), [](const FileInfo &a, const FileInfo &b) -> bool{
		return a.m_RelativeFileName < b.m_RelativeFileName;
	});

	for (auto &it : files){
		if (it.m_IsFolder)
			continue;

		string path;
		FileSystem::DataSubPaths.Translate(path, it.m_RelativeFileName, DataPath::Scripts);
		RegisterScript(path);
	}

	GetDataMgr()->NotifyResourcesChanged();
	AddLog(Debug, "Finished looking for scripts");
}

void cScriptEngine::LoadAllScripts() {
	JobQueue::QueueJob([this]() { LoadAllScriptsImpl(); });
}

void cScriptEngine::RegisterScript(string Name) {
	ScriptCode *last;
	{
		ScriptCode sc_value;
		sc_value.Name.swap(Name);
		sc_value.Type = ScriptCode::Source::File;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc_value));
		last = &m_ScriptCodeList.back();
	}

	ScriptCode &sc = *last;
	
	if (sc.Data.empty()) {
		StarVFS::ByteTable data;
		if (!GetFileSystem()->OpenFile(sc.Name, DataPath::Root, data)) {
			AddLog(Warning, "Unable to open script file " << sc.Name);
			return;
		}
		sc.Data = string(data.get(), data.size());
		AddLogf(Debug, "Loaded script code: %s", sc.Name.c_str());
	}

	if (m_Script) {
		try {
			m_Script->LoadCode(sc.Data.c_str(), sc.Data.length(), sc.Name.c_str());
		}
		catch (...) {
			AddLog(Error, "Unexpected exception while loading script code!");
		}
	}
}

void cScriptEngine::SetCode(const string& ChunkName, string Code) {
	ScriptCode *item = nullptr;
	LOCK_MUTEX(m_Mutex);
	{
		for (auto &it : m_ScriptCodeList) {
			if (it.Name == ChunkName) {
				item = &it;
				break;
			}
		}

		if(!item) {
			AddLogf(Debug, "Chunk '%s' not found. Creating new", ChunkName.c_str());
			ScriptCode sc;
			sc.Name = ChunkName;
			sc.Type = ScriptCode::Source::Code;
			m_ScriptCodeList.emplace_back(std::move(sc));
			item = &m_ScriptCodeList.back();
		}
	}

	ScriptCode &sc = *item;
	sc.Data.swap(Code);
	AddLogf(Debug, "Exchanged code for chunk '%s'", ChunkName.c_str());
}

//---------------------------------------------------------------------------------------

void cScriptEngine::LoadCode(string code) {
	ScriptCode *last;
	{
		ScriptCode sc;
		sc.Type = ScriptCode::Source::Code;
		sc.Data.swap(code);
		char buffer[64];
		sprintf(buffer, "Code_%d", m_ScriptCodeList.size());
		sc.Name = buffer;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc));
		last = &m_ScriptCodeList.back();
	}

	try {
		LOCK_MUTEX(m_Mutex);
		if (m_Script)
			m_Script->LoadCode(last->Data.c_str(), last->Data.length(), last->Name.c_str());
	}
	catch (...) {
		AddLog(Error, "Unexpected exception while broadcasting script code!");
	}

	GetDataMgr()->NotifyResourcesChanged();
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::ConstructScript() {
	LOCK_MUTEX(m_Mutex);

	if (m_Script)
		return true;

	AddLog(Debug, "Constructing script object");
	auto s = std::make_shared<Script>();
	if(!s->Initialize()) {
		AddLog(Error, "Unable to initialize script instance!");
		s.reset();
		return false;
	}

	for (auto &it : m_ScriptCodeList) {
		s->LoadCode(it.Data.c_str(), it.Data.length(), it.Name.c_str());
	}
	m_Script.swap(s);

	AddLog(Debug, "Construction finished");
	return true;
}

bool cScriptEngine::DestroyScript() {
	LOCK_MUTEX(m_Mutex);
	if (!m_Script) {
		//unexpected case, silently ignore
		return true;
	}

	AddLog(Debug, "Destroying script object");
	SharedScript s;
	s.swap(m_Script);
	if (!s->Finalize()) {
		AddLog(Warning, "An error has occur during finalization of script instance!");
	}

	s.reset();

	AddLog(Debug, "Destruction finished");
	return true;
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::InitializeScriptProxy(ScriptProxy &proxy, SharedScript& ptr) {
	AddLog(Hint, "Thread requested script instance");
	LOCK_MUTEX(m_Mutex);

	if (!m_Script) {
		return false;
	}

	ptr = m_Script;
	m_ScriptList.push_back(proxy.shared_from_this());

	return true;
}

bool cScriptEngine::FinalizeScriptProxy(ScriptProxy &proxy, SharedScript& ptr) {
	AddLog(Hint, "Thread returned script instance");
//	{
//		LOCK_MUTEX(m_Mutex);
//		for (auto &it: m_ScriptList)
//			if (it.first == ptr.get()) {
//				it.second = nullptr;
//				break;
//			}
//	}
	ptr.reset();

	return true;
}

//---------------------------------------------------------------------------------------

void cScriptEngine::EnumerateScripts(EnumerateFunc func) {
	LOCK_MUTEX(m_Mutex);
	for (auto &it : m_ScriptCodeList)
		func(it);
}

//---------------------------------------------------------------------------------------

#ifdef DEBUG_DUMP
void cScriptEngine::DumpScripts(std::ostream &out) {
	out << "Registered scripts:\n";
	{
		LOCK_MUTEX(m_Mutex);
		for (auto &it : m_ScriptCodeList) {
			out << "\t" << it.Name << "  length: " << it.Data.length() << " bytes\n";
		}
	}
	out << "\n";
}
#endif

} //namespace Scripts
} //namespace Core
