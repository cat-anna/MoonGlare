#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include "GlobalContext.h"

namespace Core {
namespace Scripts {

GABI_IMPLEMENT_CLASS_SINGLETON(cScriptEngine)
RegisterApiInstance(cScriptEngine, &cScriptEngine::Instance, "ScriptEngine");
RegisterApiDerivedClass(cScriptEngine, &cScriptEngine::RegisterScriptApi);

cScriptEngine::cScriptEngine() :
		cRootClass(),
		m_Flags(0) {
	SetThisAsInstance();

	SetPerformanceCounterOwner(ScriptInstancesConstructed);
	SetPerformanceCounterOwner(ScriptInstancesDestroyed);
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
		.addFunction("BcastCode", &ThisClass::BroadcastCode)
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
	for (auto &it : m_ScriptList) {
		Script *s = it.first;
		GetEngine()->PushSynchronizedAction([s] {
			s->CollectGarbage();
		});
	}	
}

void cScriptEngine::PrintMemoryInfo() {
	for (auto &it : m_ScriptList) {
		Script *s = it.first;
		GetEngine()->PushSynchronizedAction([s] {
			s->PrintMemoryUsage();
		});
	}	
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::Initialize() {
	if (IsReady()) return true;

	new GlobalContext();
	GlobalContext::Instance()->Initialize();

	SetReady(true);
	return true;
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
		auto &fr = m_ScriptList.front();
		if (fr.second) {
			auto ptr = fr.second->DropScriptOwnership();
			if (!ptr) {
				m_ScriptList.pop_front();
				continue;//highly unexpected case
			}
			DestroyScript(ptr);
		} else {
			m_ScriptList.pop_front();
		}
	}
}

void cScriptEngine::LoadAllScriptsImpl() {
	FileSystem::FileTable files;
	if (!GetFileSystem()->EnumerateFolder(DataPath::Scripts, files)) {
		AddLog(Error, "Unable to look for scripts!");
	}

	for (auto &it: files)
		switch (it.Type) {
		case FileSystem::FileType::File:{
			string path;
			FileSystem::DataSubPaths.Translate(path, it.Name, DataPath::Scripts);
			RegisterScript(path);
			break;
		}
		case FileSystem::FileType::Directory:
			AddLog(TODO, "Recursive search for scripts");
			break;
		default:
			AddLog(InvalidEnum, it.Type);
			break;
		}

	GetDataMgr()->NotifyResourcesChanged();
	AddLog(Debug, "Finished looking for sounds");
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
		auto file = GetFileSystem()->OpenFile(sc.Name, DataPath::Root);
		if (!file) {
			AddLog(Warning, "Unable to open script file " << sc.Name);
			return;
		}
		sc.Data = string(file->GetFileData(), file->Size());
		AddLogf(Debug, "Loaded script code: %s", sc.Name.c_str());
	}

	if (m_ScriptList.empty())
		return;

	for (auto &it : m_ScriptList) {
		Script *s = it.first;
		GetEngine()->PushSynchronizedAction([s, sc] {
			try {
				s->LoadCode(sc.Data.c_str(), sc.Data.length(), sc.Name.c_str());
			}
			catch (...) {
				AddLog(Error, "Unexpected exception while loading script code!");
			}
		});
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
	AddLogf(Debug, "Excanged code for chunk '%s'", ChunkName.c_str());
}

//---------------------------------------------------------------------------------------

void cScriptEngine::BroadcastCode(string code) {
	ScriptCode *last;
	{
		ScriptCode sc;
		sc.Type = ScriptCode::Source::Code;
		sc.Data.swap(code);
		char buffer[64];
		sprintf(buffer, "BroadcastedCode_%d", m_ScriptCodeList.size());
		sc.Name = buffer;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc));
		last = &m_ScriptCodeList.back();
	}

	for (auto &it : m_ScriptList) {
		Script *s = it.first;
		GetEngine()->PushSynchronizedAction([last, s] {
			try {
				s->LoadCode(last->Data.c_str(), last->Data.length(), last->Name.c_str());
			}
			catch (...) {
				AddLog(Error, "Unexpected exception while broadcasting script code!");
			}
		});
	}	 

	GetDataMgr()->NotifyResourcesChanged();
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::ConstructScript(UniqueScript &ptr) {
	if (!IsReady()) {
		AddLog(Error, "Script engine is not ready!");
		return false;
	}
	AddLog(Debug, "Constructing script object");
	UniqueScript s = std::make_unique<Script>();
	if(!s->Initialize()) {
		AddLog(Error, "Unable to initialize script instance!");
		s.reset();
		return false;
	}
	ptr.swap(s);
	{
		LOCK_MUTEX(m_Mutex);
		auto sx = ptr.get();
		m_ScriptList.push_back(std::make_pair(sx, (ScriptProxy*)nullptr));
		for (auto &it : m_ScriptCodeList) {
			ptr->LoadCode(it.Data.c_str(), it.Data.length(), it.Name.c_str());
		}
	}
	AddLog(Debug, "Construction finished");
	IncrementPerformanceCounter(ScriptInstancesConstructed);
	return true;
}

bool cScriptEngine::DestroyScript(UniqueScript &ptr) {
	if (!ptr) {
		//unexpected case, silently ignore
		return true;
	}
	AddLog(Debug, "Destroying script object");
	{
		LOCK_MUTEX(m_Mutex);
		m_ScriptList.remove_if([&ptr](const std::pair<Script*, ScriptProxy*>& item)->bool {
			return item.first == ptr.get();
		});
	}
	if (!ptr->Finalize()) {
		AddLog(Warning, "An error has occur during finalization of script instance!");
	}
	ptr.reset();

	AddLog(Debug, "Destruction finished");
	IncrementPerformanceCounter(ScriptInstancesDestroyed);
	return true;
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::InitializeScriptProxy(ScriptProxy &proxy, UniqueScript& ptr) {
	AddLog(Hint, "Thread requested script instance");
	LOCK_MUTEX(m_Mutex);
	if (!ConstructScript(ptr))
		return false;

	ptr->SetOwnerProxy(&proxy);
	for (auto &it: m_ScriptList)
		if (it.first == ptr.get()) {
			it.second = &proxy;
			break;
		}
	return true;
}

bool cScriptEngine::FinalizeScriptProxy(ScriptProxy &proxy, UniqueScript& ptr) {
	AddLog(Hint, "Thread returned script instance");
	{
		LOCK_MUTEX(m_Mutex);
		for (auto &it: m_ScriptList)
			if (it.first == ptr.get()) {
				it.second = nullptr;
				break;
			}
	}
	DestroyScript(ptr);
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
