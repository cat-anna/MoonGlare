#pragma once

#include "ecs/component_interface.hpp"
#include "ecs/system_interface.hpp"
#include "scenes_manager_interface.hpp"
#include <async_loader.hpp>
#include <gsl/gsl>
#include <memory>
#include <mutex>
#include <readonly_file_system.h>
#include <stepable_interface.hpp>
#include <string_view>
#include <unordered_map>

namespace MoonGlare::SceneManager {

class ScenesManager : public iScenesManager, public iStepableObject {
public:
    ScenesManager(gsl::not_null<iReadOnlyFileSystem *> _filesystem, gsl::not_null<iAsyncLoader *> _async_loader,
                  gsl::not_null<ECS::iSystemRegister *> _system_register,
                  gsl::not_null<ECS::iComponentRegister *> _component_register);
    ScenesManager(gsl::not_null<iReadOnlyFileSystem *> _filesystem,
                  std::unique_ptr<iSceneInstanceFactory> _scene_factory);
    ~ScenesManager() override;

    // iScenesManager
    iSceneInstance *CreateScene(const std::string &resource_name, std::string scene_name) override;

    //iStepableObject
    void DoStep(double time_delta) override;
#if 0
    // iSceneManager
    void Initialize(const SceneConfiguration *configuration) override;
    void Finalize() override;
    void PostSystemInit() override;
    void PreSystemStart() override;
    bool IsScenePending() const override { return pendingScene.load() != nullptr; }

    void HandleEvent(const Resources::ResourceLoaderEvent &event);
    void HandleEvent(const SetSceneEvent &event);
    void HandleEvent(const SetSceneChangeFenceEvent &event);
    void HandleEvent(const SceneStateChangeEvent &event);

    void Step(Core::MoveConfig &config);

    void DumpAllDescriptors();

    static void RegisterScriptApi(::ApiInitializer &api);
#endif
protected:
    std::unique_ptr<iSceneInstanceFactory> const scene_factory;
    std::recursive_mutex mutex;

    struct SceneDescriptor {
        FileResourceId resource_id;
        std::string name;
    };
    std::unordered_map<std::string, SceneDescriptor> scene_descriptors;

    std::unordered_map<std::string, std::unique_ptr<iSceneInstance>> scene_instances;

    iSceneInstance *loading_scene = nullptr;
    iSceneInstance *current_scene = nullptr; // currently set and simulated scene
    iSceneInstance *next_scene = nullptr;    // Scene to be activated at next loop

    void CacheScenes(iReadOnlyFileSystem *const filesystem);
#if 0
    using SceneDescMap = std::unordered_map<string, SceneDescriptor *>;
    using SceneInstMap = std::unordered_map<string, UniqueSceneInstance>;

    InterfaceMap &interfaceMap;
    Component::EventDispatcher *eventDispatcher = nullptr;
    Component::EntityManager *entityManager = nullptr;
    PrefabManager *prefabManager = nullptr;
    const SceneConfiguration *sceneConfiguration = nullptr;

    SceneInstMap sceneInstances;

    std::vector<UniqueSceneDescriptor> knownSceneDescriptors;
    SceneDescMap descriptorsByName;
    int resourceLoadRevision = 0;

    SceneDescriptor *CreateDescriptor(StarVFS::FileID fid, const std::string &name);
    SceneDescriptor *FindDescriptor(const std::string &name);

    SceneInstance *CreateScene(const std::string &descName, const std::string &alias = "");
    SceneInstance *FindSceneInstance(const std::string &sceneName);
    bool LoadSceneData(SceneDescriptor *descriptor, SceneInstance *instance);

    void ChangeScene(Core::MoveConfig &config);
    void UpdatePendingSceneFence(const std::string_view fenceName, bool state);
    void ProcessPreviousScene(SceneInstance *instance);
    void DropSceneInstance(SceneInstance *instance);
#endif
};

} // namespace MoonGlare::SceneManager
