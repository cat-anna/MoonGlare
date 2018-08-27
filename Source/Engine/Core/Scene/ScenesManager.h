#pragma once

#include <unordered_set>

#include <Foundation/InterfaceMap.h>

#include "iSceneManager.h"

#include "Renderer.Events.h"

namespace MoonGlare::Core::Scene {

struct SetSceneEvent;

struct SceneInstance;
struct SceneDescriptor;
using UniqueSceneDescriptor = std::unique_ptr<SceneDescriptor>;
using UniqueSceneInstance = std::unique_ptr<SceneInstance>;

class ScenesManager : public iSceneManager {
public:
    ScenesManager(InterfaceMap &ifaceMap);
    virtual ~ScenesManager();

//iSceneManager
    void Initialize(const SceneConfiguration *configuration)  override;
    void Finalize()                                           override;
    void PostSystemInit()                                     override;
    void PreSystemStart()                                     override;
    bool IsScenePending() const override { return pendingScene.load() != nullptr; }

    void HandleEvent(const Renderer::RendererResourceLoaderEvent &event);
    void HandleEvent(const SetSceneEvent &event);

    void Step(const Core::MoveConfig &config);

    //bool LoadScene(const std::string &SID); //async LoadNextScene
    //bool DropSceneState(const std::string &SID);
    //bool SetSceneStateful(const std::string &SID, bool value);

    //double GetSceneTime() const {
        //if (!m_CurrentScene)
            //return 0.0;
        //return std::chrono::duration<double>(std::chrono::steady_clock::now() - sceneStartTime).count();
    //}

    //void SetSceneChangeFence(SceneChangeFence type, bool value);

#ifdef DEBUG_DUMP
    //void DumpAllDescriptors(std::ostream& out);
#endif
    static void RegisterScriptApi(::ApiInitializer &api);
protected:
    using SceneDescMap = std::unordered_map<string, SceneDescriptor*>;
    using SceneInstMap = std::unordered_map<string, UniqueSceneInstance>;

    InterfaceMap &interfaceMap;
    Component::EventDispatcher *eventDispatcher = nullptr;
    const SceneConfiguration *sceneConfiguration = nullptr;
    std::recursive_mutex mutex;

    SceneInstance* loadingScene = nullptr;
    SceneInstance* currentScene = nullptr;                 //currently set and simulated scene
    std::atomic<SceneInstance*> nextScene = nullptr;       //Scene to be activated at next loop
    std::atomic<SceneInstance*> pendingScene = nullptr;    //Scene which is under initialization
    
    SceneInstMap sceneInstances;

    std::vector<UniqueSceneDescriptor> knownSceneDescriptors;
    SceneDescMap descriptorsByName;

    SceneDescriptor* CreateDescriptor(StarVFS::FileID fid, const std::string &name);
    SceneDescriptor* FindDescriptor(const std::string &name);

    SceneInstance* CreateScene(const std::string &descName, const std::string &alias = "");
    SceneInstance* FindSceneInstance(const std::string &sceneName);
    bool LoadSceneData(SceneDescriptor *descriptor, SceneInstance* instance);

    void ChangeScene();
    void UpdatePendingSceneFence(const std::string &fenceName, bool state);

//old
    //using SceneDescriptorTable = std::vector<UniqueSceneDescriptor>;
    //SceneInstance *currentScene = nullptr;
    //SceneDescriptor *m_CurrentSceneDescriptor = nullptr;
    //SceneDescriptor *m_NextSceneDescriptor = nullptr;
    //SceneDescriptor *m_LoadingSceneDescriptor = nullptr;

    //SceneSIDMap m_SIDMap;
    //SceneDescriptorTable m_DescriptorTable;

    //bool m_LoadingInProgress = false;
    //Renderer::SharedAsyncLoaderObserver loaderObserver;

    //bool LoadNextScene(const std::string &SID);					
    //bool LoadNextScene(SceneDescriptor *descriptor);			

    //void ProcessPreviousScene(SceneDescriptor *descriptor);
    
    /** Manager shall be already locked */
};

} //namespace Scene::Core::MoonGlare 
