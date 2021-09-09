#include "scene_manager/scenes_manager.hpp"
#include "scene_instance.hpp"
#include "scene_manager/configuration.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
#include <unordered_set>

using namespace std::string_literals;

namespace MoonGlare::SceneManager {

namespace {} // namespace

//----------------------------------------------------------------------------------

ScenesManager::ScenesManager(gsl::not_null<iReadOnlyFileSystem *> _filesystem,
                             std::unique_ptr<iSceneInstanceFactory> _scene_factory)
    : scene_factory(std::move(_scene_factory)) {

    CacheScenes(_filesystem);
}

ScenesManager::~ScenesManager() {
}

void ScenesManager::CacheScenes(iReadOnlyFileSystem *filesystem) {
    FileInfoTable fit;
    if (!filesystem->FindFilesByExt(".sdx", fit)) {
        AddLog(Error, "Failed to search for scene files");
        return;
    }
    AddLog(Debug, fmt::format("Found {} scene descriptor", fit.size()));

    for (auto &item : fit) {
        std::string_view name = item.file_name;
        name.remove_suffix(4); //4 - length(".sdx")

        AddLog(Debug,
               fmt::format("Got scene descriptor {} : resid={:x}", name, item.file_resource_id));

        scene_descriptors[std::string(name)] = SceneDescriptor{
            // .resource_id=
            item.file_resource_id,
            // .name =
            std::string(name),
        };
    }
}

//----------------------------------------------------------------------------------

iSceneInstance *ScenesManager::CreateScene(const std::string &resource_name,
                                           std::string scene_name) {
    if (scene_name.empty()) {
        scene_name = resource_name;
    }

    auto descriptor = scene_descriptors.find(resource_name);
    if (descriptor == scene_descriptors.end()) {
        AddLog(Warning, fmt::format("Scene descriptor {} does not exists", resource_name));
        return nullptr;
    }

    if (scene_instances.find(scene_name) != scene_instances.end()) {
        AddLog(Warning, fmt::format("Scene {} already exists, cannot crate new one", scene_name));
        return nullptr;
    }

    AddLog(Debug, fmt::format("Creating scene {} of name {}", resource_name, scene_name));

    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto new_scene_instance =
        scene_factory->CreateSceneInstance(scene_name, descriptor->second.resource_id);

    // scene.descriptor = desc;
    // scene.entityManager = entityManager;
    // scene.eventDispatcher = eventDispatcher;
    // eventDispatcher->AddSubDispatcher(&scene.subsystemManager.GetEventDispatcher()); //TODO:!!
    // pendingScene = &scene;

    //TODO: loading scenes is synchronous

    // if (!LoadSceneData(desc, &scene)) {
    //     AddLogf(Error, "Failed to load scene '%s' from descriptor '%s'", sceneName.c_str(), descName.c_str());
    //     pendingScene = nullptr;
    //     return nullptr;
    // }

    // scene.SendState(SceneState::Paused);

    auto raw_ptr = new_scene_instance.get();
    scene_instances[scene_name] = std::move(new_scene_instance);

    if (scene_name == kLoadingSceneName) {
        loading_scene = raw_ptr;
    }

    if (current_scene == nullptr && next_scene == nullptr) {
        //TODO?
        next_scene = raw_ptr;
    }

    return raw_ptr;
}

//----------------------------------------------------------------------------------

void ScenesManager::DoStep(double time_delta) {
    if (next_scene != nullptr && next_scene->ReadyForActivation()) {
        AddLog(Hint, fmt::format("Changing scene from '{}' to '{}'",
                                 (current_scene ? current_scene->GetSceneName() : "NULL"s),
                                 (next_scene ? next_scene->GetSceneName() : "NULL"s)));

        std::lock_guard<std::recursive_mutex> lock(mutex);

        // SceneInstance::TimePoint currentTime = SceneInstance::TimePoint::clock::now();

        if (current_scene != nullptr) {
            // currentScene->PauseTime(config, currentTime);
            // currentScene->SendState(SceneState::Paused);
        } else {
            // eventDispatcher->Queue(SetSceneEvent(sceneConfiguration->firstScene.c_str()));
        }

        auto *prev_Scene = current_scene;
        current_scene = next_scene;
        next_scene = nullptr;

        // ProcessPreviousScene(prevScene);

        if (current_scene) {
            // currentScene->RestoreTime(config, currentTime);
            // currentScene->SendState(SceneState::Started);
            // config.deferred->SetStaticFog(currentScene->staticFog);
        }
    }

    if (current_scene != nullptr) {
        current_scene->DoStep(time_delta);
    }
}

//----------------------------------------------------------------------------------

#if 0

void ScenesManager::Initialize(const SceneConfiguration *configuration) {
    sceneConfiguration = configuration;
    eventDispatcher->Register<Resources::ResourceLoaderEvent>(this);
    eventDispatcher->Register<SetSceneEvent>(this);
    eventDispatcher->Register<SetSceneChangeFenceEvent>(this);
    eventDispatcher->Register<SceneStateChangeEvent>(this);
}

//----------------------------------------------------------------------------------

void ScenesManager::HandleEvent(const SceneStateChangeEvent &event) {
    if (SceneInstance *inst = FindSceneInstance(event.sceneName); inst) {
        inst->SetFenceState(std::string(SceneNotReadyFenceName), event.state != SceneState::Paused);
    }
}

void ScenesManager::HandleEvent(const Resources::ResourceLoaderEvent &event) {
    if (resourceLoadRevision > event.revision)
        return;
    resourceLoadRevision = event.revision;
    UpdatePendingSceneFence(ResourceFenceName, event.busy);
}

void ScenesManager::HandleEvent(const SetSceneEvent &event) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (currentScene)
        currentScene->wantsToSleep = event.suspendCurrent;

    if (SceneInstance *inst = FindSceneInstance(event.sceneName); inst) {
        if (inst->activeFences.empty()) {
            nextScene = inst;
            pendingScene = nullptr;
        } else {
            nextScene = loadingScene;
            pendingScene = inst;
        }
        return;
    }

    CreateScene(event.GetTypeName(), event.sceneName);
}

void ScenesManager::HandleEvent(const SetSceneChangeFenceEvent &event) {
    std::string fenceName = "script." + std::string(event.fence);
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!event.sceneName.empty()) {
        if (SceneInstance *inst = FindSceneInstance(event.sceneName); inst) {
            if (inst == currentScene)
                return; //TODO: decide how to properly handle this case

            if (inst == nextScene) {
                nextScene = loadingScene;
                pendingScene = inst;
            }
            if (inst == pendingScene)
                UpdatePendingSceneFence(fenceName, event.active);
            else
                inst->SetFenceState(fenceName, event.active);
        }
    } else
        UpdatePendingSceneFence(fenceName, event.active);
}

//----------------------------------------------------------------------------------

SceneInstance *ScenesManager::FindSceneInstance(const std::string &sceneName) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto sid_it = sceneInstances.find(sceneName);
    if (sid_it != sceneInstances.end()) {
        AddLogf(Debug, "Scene instance exits: sid:%s", sceneName.c_str());
        return sid_it->second.get();
    }
    AddLogf(Debug, "Scene instance DOES NOT exits: sid:%s", sceneName.c_str());
    return nullptr;
}

//----------------------------------------------------------------------------------

void ScenesManager::UpdatePendingSceneFence(const std::string_view fenceName, bool state) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto pending = pendingScene.load();
    if (pending) {
        bool ready = pending->SetFenceState(std::string(fenceName), state);
        static bool d = true;
        if (d && ready) {
            //d = false;
            pendingScene = nullptr;
            nextScene = pending;
        }
    }
}

void ScenesManager::ProcessPreviousScene(SceneInstance *instance) {
    if (!instance)
        return;

    if (instance == loadingScene)
        return;

    if (instance->wantsToSleep)
        //TODO: some standby mechanics
        return;

    DropSceneInstance(instance);
}

void ScenesManager::DropSceneInstance(SceneInstance *instance) {
    if (!instance)
        return;

    std::lock_guard<std::recursive_mutex> lock(mutex);
    instance->SendState(SceneState::BeforeDestruction);
    instance->Finalize();
    sceneInstances.erase(instance->sceneName);
}

#endif

} // namespace MoonGlare::SceneManager
