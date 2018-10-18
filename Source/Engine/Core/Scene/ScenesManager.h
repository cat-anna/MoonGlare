#pragma once

#include <unordered_set>

#include <Foundation/InterfaceMap.h>

#include "iSceneManager.h"

#include "Renderer.Events.h"

namespace MoonGlare::Core::Scene {

struct SetSceneEvent;
struct SetSceneChangeFenceEvent;
struct SceneStateChangeEvent;

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
    void HandleEvent(const SetSceneChangeFenceEvent &event);
    void HandleEvent(const SceneStateChangeEvent &event);

    void Step(Core::MoveConfig &config);

    void DumpAllDescriptors();

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
    int resourceLoadRevision = 0;

    SceneDescriptor* CreateDescriptor(StarVFS::FileID fid, const std::string &name);
    SceneDescriptor* FindDescriptor(const std::string &name);

    SceneInstance* CreateScene(const std::string &descName, const std::string &alias = "");
    SceneInstance* FindSceneInstance(const std::string &sceneName);
    bool LoadSceneData(SceneDescriptor *descriptor, SceneInstance* instance);

    void ChangeScene(Core::MoveConfig & config);
    void UpdatePendingSceneFence(const std::string_view fenceName, bool state);
    void ProcessPreviousScene(SceneInstance *instance);
    void DropSceneInstance(SceneInstance *instance);
};

} //namespace Scene::Core::MoonGlare 
