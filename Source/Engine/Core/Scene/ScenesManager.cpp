/*
 * ScenesManager.cpp
 *
 *  Created on: 16-12-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

#include "BackstageScene.h"
#include "LoadingBaseScene.h"
#include "DefaultLoadingScene.h"

#include <Engine/Core/Engine.h>

#include <Engine/BaseResources.h>

namespace MoonGlare {
namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(ScenesManager)
RegisterApiDerivedClass(ScenesManager, &ScenesManager::RegisterScriptApi);

struct ScenesManagerTimers {
	enum {
		SceneTimeout = 1,
	};
};

ScenesManager::ScenesManager():
		m_Flags(0),
		m_NextSceneRegister(nullptr) {
	m_EventProxy.set(new EventProxy<ThisClass, &ThisClass::HandleTimer>(this));
	SetThisAsInstance();
}

ScenesManager::~ScenesManager() {
	Finalize();
}

//----------------------------------------------------------------------------------

void ScenesManager::RegisterScriptApi(ApiInitializer &api) {
}

//----------------------------------------------------------------------------------

bool ScenesManager::Initialize() { 
	{
//register backstage scene
		ciScene *ptr = new BackstageScene();
		auto &sd = AllocDescriptor(ptr->GetName(), SceneType::Backstage);
		sd.Class = BackstageScene::GetStaticTypeInfo()->GetName();
		sd.ptr.reset(ptr);
		sd.ptr->Initialize();
		m_SceneStack.push_back(&sd);
	}
	{
//register default loading scene
		//ciScene *ptr = new EngineLoadScene();
		//auto &sd = AllocDescriptor(LoadingSceneName, SceneType::External);
		//sd.Class = DefaultLoadingScene::GetStaticTypeInfo()->GetName();
		//sd.ptr.reset(ptr);
		//sd.ptr->Initialize();
		//m_SceneStack.push_back(&sd);
	}
	return true;
}

bool ScenesManager::Finalize() { 
	m_SceneStack.clear();
	m_SceneList.clear();
	return true;
}

//----------------------------------------------------------------------------------

#ifdef DEBUG_DUMP

void ScenesManager::DumpAllDescriptors(std::ostream& out) {
	out << "Scene descriptors:\n";
	for (auto &it : m_SceneList) {
		auto &sd = it.second;
		char buf[128];
		sprintf(buf, "%40s [class %-20s]\n", 
				sd.Name.c_str(), sd.Class.c_str());
		out << buf;
	}
	out << "\n";
}

#endif // DEBUG_DUMP

//----------------------------------------------------------------------------------

int ScenesManager::HandleTimer(int TimerID) {
	switch (TimerID) {
	case ScenesManagerTimers::SceneTimeout: {
		//SetSceneLoadingTimedOut(true);
		JobQueue::QueueJob([this] { LoadingSceneTimedOutJob(); });
		break;
	}
	default:
		AddLogf(Warning, "SceneManager recived undefined TimerID: %d", TimerID);
	}
	return 0;
}

//----------------------------------------------------------------------------------

void ScenesManager::LoadingSceneTimedOutJob() {
	LOCK_MUTEX(m_Lock);
	if (IsNextSceneLoaded()) {
		AddLog(Debug, "Scene loading timeout expired!");
		return;
	}
	SetSceneLoadingTimedOut(true);

	AddLog(Hint, "Loading scene timed out!");

	auto *desc = GetSceneDescriptor(Configuration::BaseResources::FallbackLoadScene::get());
	if (!desc) {
		AddLog(Error, "Unable to find loading scene descriptor");
		return;
	}

	auto *scene = GetSceneInstance(desc);
	if (!scene) {
		AddLog(Error, "Unable to create loading scene instance");
		return;
	}

	m_NextSceneRegister = scene;
	GetEngine()->ChangeScene();
}

void ScenesManager::LoadSceneJob(const string& Name, int Param, EventProxyPtr proxy) {
	auto *desc = GetSceneDescriptor(Name);
	if (!desc) {
		AddLogf(Error, "Unable to find descriptor for scene: '%s'", Name.c_str());
		SetNextSceneLoaded(false);
		SetNextScenePending(false);
		auto evHandler = proxy.lock();
		if (evHandler) {
			evHandler->InternalEvent(Events::InternalEvents::SceneLoadingFinished, Param);
		}
		return;
	}

	ciScene *s;
	if (desc->ptr) {
		s = desc->ptr.get();
	} else {
		if (proxy.expired()) {
			GetEngine()->SetProxyTimer(GetEventProxy(), 1.0f/*StaticSettings::Scenes::GetSceneLoadTimeOut()*/, ScenesManagerTimers::SceneTimeout, false);
		}
		s = GetSceneInstance(desc);
	}

	while (IsSceneLoadingTimedOut()) {
		//wait until loding scene has been set
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	LOCK_MUTEX(m_Lock);

	GetEngine()->KillProxyTimer(GetEventProxy(), ScenesManagerTimers::SceneTimeout);
	if (!s) {
		SetNextScenePending(false);
		SetNextSceneLoaded(false);
	} else {
		m_NextSceneRegister = s;
		SetNextSceneLoaded(true);
	}
	auto evHandler = proxy.lock();

	if (!evHandler)
		GetEngine()->ChangeScene();
	else
		evHandler->InternalEvent(Events::InternalEvents::SceneLoadingFinished, Param);
}

//----------------------------------------------------------------------------------

ciScene* ScenesManager::GetNextScene() {
	{
		LOCK_MUTEX(m_Lock);
		if (IsSceneLoadingTimedOut()) {
			if (!m_NextSceneRegister)
				return nullptr;
			SetSceneLoadingTimedOut(false);
			auto *b = m_NextSceneRegister; 
			m_NextSceneRegister = nullptr;
			return b;
		}

		if (IsNextSceneLoaded()) {
			if (!m_NextSceneRegister)
				return nullptr;
			SetNextScenePending(false);
			SetNextSceneLoaded(false);
			auto *b = m_NextSceneRegister; 
			m_NextSceneRegister = nullptr;
			return b;
		}

		if (IsNextScenePending())
			return nullptr;
	}
	return PopScene();
}

void ScenesManager::ScenePrepeareImpl(const string& Name, int Param, EventProxyPtr proxy) {
	LOCK_MUTEX(m_Lock);
	if (IsNextScenePending()) {
//		AddLog(FixMe, "Setting next scene while next scene is pending is not implemented");
		AddLog(Error, "Cannot set next scene while next scene is pending!");
		GetEngine()->Abort();
	}

	m_NextSceneRegister = nullptr;
	SetNextScenePending(true);
	JobQueue::QueueJob([this, Name, Param, proxy] { LoadSceneJob(Name, Param, proxy); });
}

void ScenesManager::AsyncSetNextScene(const string& Name, EventProxyPtr proxy, int Param) {
	ScenePrepeareImpl(Name, Param, proxy);
}

void ScenesManager::SetNextScene(const string& Name, int Param) {
	ScenePrepeareImpl(Name, Param, EventProxyPtr());
}

void ScenesManager::PushScene(ciScene *scene) {
	if (!scene) return;

	auto *descr = GetSceneDescriptor(scene->GetName());
	if (!descr) {
		AddLogf(Error, "Descriptor for scene '%s' not found!", scene->GetName().c_str());
		return;
	}
	LOCK_MUTEX(m_Lock);
	switch (scene->GetSceneState()) {
	case SceneState::Finished:
		AddLogf(Debug, "Destroying scene '%s', because scene is finished.", scene->GetName().c_str());
		descr->KillScene();
		return;
	default:
		break;
	}
	//descr->Suspend();
	m_SceneStack.push_back(descr);
}

ciScene* ScenesManager::PopScene() {
	if (m_SceneStack.empty()) {
		AddLog(Error, "Scene stack is empty!");
		return nullptr;
	}
	SceneDescriptor *descr;
	{
		LOCK_MUTEX(m_Lock);
		descr = m_SceneStack.back();
		m_SceneStack.pop_back();
	}
	return descr->ptr.get();
}

void ScenesManager::ClearSceneStack() {
	AddLogf(Debug, "Received request to clear scene stack!");
	LOCK_MUTEX(m_Lock);

	do {
		auto desc = m_SceneStack.back();
		if (desc->Type == SceneType::Backstage)
			break;
		
		auto s = PopScene();
		s->SetSceneState(SceneState::Finished);
		PushScene(s);
	} while (true);
}

void ScenesManager::PopScenes(int count) {
	AddLogf(Debug, "Received request to pop %d scenes", count);
	LOCK_MUTEX(m_Lock);

	for (int i = 0; i < count; ++i) {
		auto desc = m_SceneStack.back();
		if (desc->Type == SceneType::Backstage) {
			AddLogf(Error, "Unable to pop %d scenes: backstage has been found! Cleared %s scenes", count, i);
			break;
		}

		auto s = PopScene();
		s->SetSceneState(SceneState::Finished);
		PushScene(s);
	} 
}

void ScenesManager::ClearScenesUntil(const string& Name) {
	AddLogf(Debug, "Received request to remove all scenes until '%s'", Name.c_str());
	LOCK_MUTEX(m_Lock);

	do {
		auto desc = m_SceneStack.back();
		if (desc->Type == SceneType::Backstage) {
			AddLogf(Error, "Unable to pop all scenes until '%s': backstage has been found!", Name.c_str());
			break;
		}
		if (desc->Name == Name) {
			break;
		}

		auto s = PopScene();
		s->SetSceneState(SceneState::Finished);
		PushScene(s);
	} while (true);
}

//----------------------------------------------------------------------------------

SceneDescriptor* ScenesManager::GetSceneDescriptor(const string &Name) {
	{
		LOCK_MUTEX(m_Lock);
		auto it = m_SceneList.find(Name);
		if (it != m_SceneList.end()) {
			if (it->second.Type == SceneType::Invalid)
				return nullptr;
			return &it->second;
		}
	}		
	FileSystem::XMLFile xml;
	if (!GetFileSystem()->OpenResourceXML(xml, Name, DataPath::Scenes)) {
		AddLogf(Error, "Unable to open xml file for scene '%s'", Name.c_str());
		return nullptr;
	}
	auto node = xml->document_element();
	const char* xmlclass = node.attribute(xmlAttr_Class).as_string(nullptr);
	auto &sd = AllocDescriptor(Name, SceneType::Invalid);
	sd.Class = xmlclass ? xmlclass : "Scene";
	sd.Type = SceneType::External;
	GetDataMgr()->NotifyResourcesChanged();
	return &sd;
}

SceneDescriptor& ScenesManager::AllocDescriptor(const string& Name, SceneType Type) {
	LOCK_MUTEX(m_Lock);
	auto p = m_SceneList.emplace(std::piecewise_construct, std::tuple<std::string>(Name), std::tuple<>());
	auto &sd = p.first->second;
	sd.Name = Name;
	sd.Type = Type;
	return sd;
}

ciScene* ScenesManager::GetSceneInstance(SceneDescriptor *descr) {
	if (!descr) {
		AddLogf(Error, "Wrong descriptor!");
		return 0;
	}
	LOCK_MUTEX(descr->Lock);
	if (descr->ptr) {
		//todo: do sth with descriptor?
		return descr->ptr.get();
	} else {
		switch (descr->Type) {
		case SceneType::External: {
			auto *s = GetDataMgr()->LoadScene(descr->Name, descr->Class);
			if (!s)
				break;
			descr->ptr.reset(s);
			s->Initialize();
			return descr->ptr.get();
		}

		case SceneType::Internal:
			AddLog(Error, "Engine scenes cannot be created that way.");
			break;
		}
	}
	AddLog(Error, "Unable to get scene instance!");
	return 0;
}

//----------------------------------------------------------------------------------

} //namespace Scene
} //namespace Core
} //namespace MoonGlare 
