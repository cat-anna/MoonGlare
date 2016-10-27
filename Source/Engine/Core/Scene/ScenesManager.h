/*
 * cSceneManager.h
 *
 *  Created on: 16-12-2013
 *      Author: Paweu
 */
#ifndef SCENESMANAGER_H_
#define SCENESMANAGER_H_

namespace MoonGlare {
namespace Core {
namespace Scene {

enum class SceneType {
	Unknown,
	Invalid,
	Backstage,
	External,
	Internal,
//max enum value
	MaxValue,
};

#define LoadingSceneName		"LoadingScene"

struct SceneDescriptor {
	string Name, Class;
	std::unique_ptr<ciScene> ptr;
	SceneType Type;
	std::mutex Lock;

	void KillScene() {
		if (ptr) {
			ptr->Finalize();
			ptr.reset();
		}
	}

	SceneDescriptor() : ptr() { }
	~SceneDescriptor() {
		KillScene();
	}

	SceneDescriptor(const SceneDescriptor&) = delete;
	SceneDescriptor& operator=(const SceneDescriptor&) = delete;
};

class ScenesManager : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(ScenesManager, cRootClass);
	DECLARE_PROTECTED_EVENT_HOLDER();
public:
	ScenesManager();
	virtual ~ScenesManager();

	bool Initialize(World *world);
	bool Finalize();

	typedef std::unordered_map<string, SceneDescriptor> SceneList;
	typedef std::list<SceneDescriptor*> SceneStack;
	bool PostSystemInit();
	bool PreSystemStart();
	bool PreSystemShutdown();

	ciScene* GetNextScene();
	void SetNextScene(const string& Name, int Param = 0);
	void AsyncSetNextScene(const string& Name, EventProxyPtr proxy, int Param = 0);
	void PushScene(ciScene *scene);
	void ClearSceneStack();
	void PopScenes(int count);
	void ClearScenesUntil(const string& Name);
	bool Step(const Core::MoveConfig &config);

	//void NotifySceneLoaded(ciScene *scene);

#ifdef DEBUG_DUMP
	void DumpAllDescriptors(std::ostream& out);
#endif

	enum class Flags : unsigned {
		NextScenePending		= 0x0001,
		SceneLoadingTimedOut	= 0x0002,
		NextSceneLoaded			= 0x0004,
	};
	DefineFlagGetter(m_Flags, Flags::NextScenePending, NextScenePending);

	static void RegisterScriptApi(::ApiInitializer &api);
protected:
	unsigned m_Flags;
	std::recursive_mutex m_Lock;

	SceneList m_SceneList;
	SceneStack m_SceneStack;
	ciScene* m_NextSceneRegister;

	World *m_World;
	ciScene* PopScene();
	ciScene* GetSceneInstance(SceneDescriptor *descr);
	int HandleTimer(int TimerID);
	void ScenePrepeareImpl(const string& Name, int Param, EventProxyPtr proxy);

	void LoadingSceneTimedOutJob();
	void LoadSceneJob(const string& Name, int Param, EventProxyPtr proxy);

	SceneDescriptor& AllocDescriptor(const string& Name, SceneType Type);
	SceneDescriptor* GetSceneDescriptor(const string &Name);

	DefineFlagSetter(m_Flags, Flags::NextScenePending, NextScenePending);
	DefineFlag(m_Flags, Flags::SceneLoadingTimedOut, SceneLoadingTimedOut);
	DefineFlag(m_Flags, Flags::NextSceneLoaded, NextSceneLoaded);
};

} //namespace Scene
} //namespace Core
} //namespace MoonGlare 

#endif //SCENEMANAGER_H_
