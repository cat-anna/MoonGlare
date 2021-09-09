#pragma once

#include "async_loader.hpp"
#include "ecs/component_array.hpp"
#include "ecs/component_interface.hpp"
#include "ecs/entity_manager.hpp"
#include "ecs/system_interface.hpp"
#include "renderer/facade.hpp"
#include "scene_manager/prefab_manager_interface.hpp"
#include "scene_manager/scenes_manager_interface.hpp"
#include <boost/container/static_vector.hpp>
#include <gsl/gsl>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_set>

namespace MoonGlare::SceneManager {

class SceneInstance : public iSceneInstance {
public:
    SceneInstance(std::string name, FileResourceId resource_id, ECS::EntityManagerId scene_id,
                  gsl::not_null<iAsyncLoader *> _async_loader,
                  gsl::not_null<ECS::iComponentRegister *> _component_register,
                  gsl::not_null<iPrefabManager *> _prefab_manager,
                  gsl::not_null<Renderer::iRenderingDeviceFacade *> _rendering_device);

    //iSceneInstance
    bool ReadyForActivation() const override;

    //iStepableObject
    void DoStep(double time_delta) override;

private:
    std::recursive_mutex mutex;
    std::unordered_set<std::string> active_fences;

    std::vector<std::unique_ptr<ECS::iSystem>> all_systems;

    boost::container::static_vector<ECS::iSystem *, ECS::kMaxStepableSystems> stepable_systems;
    ECS::ComponentArray component_array;
    ECS::EntityManager entity_manager;

    Renderer::iRenderingDeviceFacade *const rendering_device;

    //returns true if NO fence is set
    bool SetFenceState(std::string name, bool state);
    void LoadSceneContent(iPrefabManager *prefab_manager, std::string &_file_data);
};

} // namespace MoonGlare::SceneManager