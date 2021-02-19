
#include "scene_instance.hpp"
#include "scene_manager/configuration.hpp"
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>

namespace MoonGlare::SceneManager {

SceneInstance::SceneInstance(std::string name, FileResourceId resource_id, ECS::EntityManagerId scene_id,
                             gsl::not_null<iAsyncLoader *> _async_loader,
                             gsl::not_null<ECS::iComponentRegister *> _component_register,
                             gsl::not_null<iPrefabManager *> _prefab_manager)
    : iSceneInstance(std::move(name)), component_array(_component_register),
      entity_manager(scene_id, &component_array) {

    SetFenceState(kSceneFenceLoadPending, true);

    AddLog(Debug, fmt::format("Scheduling scene load '{}'", GetSceneName()));
    _async_loader->LoadFile(resource_id, [this, _prefab_manager](auto, std::string &file_data) {
        LoadSceneContent(_prefab_manager, file_data);
    });
}

bool SceneInstance::SetFenceState(std::string name, bool state) {
    AddLog(Debug, fmt::format("Scene {} fence changed {} state: {}", GetSceneName(), name, state ? "true" : "false"));
    if (!name.empty()) {
        if (state) {
            active_fences.emplace(std::move(name));
        } else {
            active_fences.erase(std::move(name));
        }
    }
    return active_fences.empty();
}

//----------------------------------------------------------------------------------

void SceneInstance::DoStep(double time_delta) {
    //TODO: too deep calltrace to step scene
    // engine_core->scene_manager->scene_instance

    for (auto item : stepable_systems) {
        item->DoStep(time_delta);
    }

    // entity_manager.DoStep(time_delta);
    // component_array.DoStep(time_delta);
}

//----------------------------------------------------------------------------------

void SceneInstance::LoadSceneContent(iPrefabManager *prefab_manager, std::string &_file_data) {
    try {
        auto scene_config = nlohmann::json::parse(_file_data);

        ECS::SystemCreateInfo sci;
        auto loaded_systems = prefab_manager->LoadSystemConfiguration(sci, scene_config[kSceneSystemsConfig]);
        all_systems.swap(loaded_systems.systems);
        stepable_systems = loaded_systems.stepable_systems;

        prefab_manager->LoadRootEntity(&entity_manager, scene_config[kSceneEntityConfig]);

        SetFenceState(kSceneFenceLoadPending, false);
    } catch (const std::exception &e) {
        AddLog(Error, fmt::format("Unable to load config for scene: '{}' : {}", GetSceneName(), e.what()));
    }
}

//----------------------------------------------------------------------------------

bool SceneInstance::ReadyForActivation() const {
    return active_fences.empty();
}

#if 0
    using TimePoint = Component::SubsystemUpdateData::TimePoint;

    Component::SubsystemManager subsystemManager;

    TimePoint localTimeStart = TimePoint::clock::now();
    double localTimeBase = 0.0;

    bool wantsToSleep = false;

    SceneDescriptor *descriptor = nullptr;
    Component::EntityManager *entityManager = nullptr;
    Component::EventDispatcher *eventDispatcher = nullptr;
    MoonGlare::Renderer::StaticFog staticFog = {};
    Entity sceneRoot;

    void PauseTime(Component::SubsystemUpdateData &sud, TimePoint timePoint) {
        localTimeBase += TimeDiff(localTimeStart, timePoint);
        localTimeStart = timePoint;
    }
    void RestoreTime(Component::SubsystemUpdateData &sud, TimePoint timePoint) {
        localTimeStart = timePoint;
        sud.localTimeBase = localTimeBase;
        sud.localTimeStart = timePoint;
    }

    void SendState(SceneState state) {
        SceneStateChangeEvent ssce;
        ssce.sceneName = sceneName;
        ssce.state = state;
        eventDispatcher->Queue<SceneStateChangeEvent>(ssce);
    }

    bool Initialize(pugi::xml_node node, PrefabManager *prefabManager) {
        AddLog(Debug, "Initializing scene: " << sceneName);

        if (!entityManager->Allocate(sceneRoot, sceneName)) {
            AddLogf(Error, "Failed to allocate scene entity!");
            return false;
        }

        if (!subsystemManager.LoadSystems(node.child("Systems"))) {
            AddLogf(Error, "Failed to load components");
            return false;
        }

        if (!subsystemManager.Initialize(sceneRoot)) {
            AddLogf(Error, "Failed to initialize component manager");
            return false;
        }

        eventDispatcher->AddSubDispatcher(&subsystemManager.GetEventDispatcher());

        prefabManager->Spawn(&subsystemManager, sceneRoot, node.child("Entities"),
                             fmt::format("scene://{}", sceneName));
        //EntityBuilder(&subsystemManager).Build(sceneRoot, sceneName.c_str(), Node.child("Entities"));
        return true;
    }

#endif

} // namespace MoonGlare::SceneManager
