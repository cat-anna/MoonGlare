#pragma once

#include "ecs/component_array.hpp"
#include "ecs/component_interface.hpp"
#include "ecs/system_interface.hpp"
#include <async_loader.hpp>
#include <boost/container/static_vector.hpp>
#include <gsl/gsl>
#include <memory>
#include <mutex>
#include <scene_manager/scenes_manager_interface.hpp>
#include <string_view>
#include <unordered_set>

namespace MoonGlare::SceneManager {

class SceneInstance : public iSceneInstance {
public:
    SceneInstance(std::string name, FileResourceId resource_id, gsl::not_null<iAsyncLoader *> _async_loader,
                  gsl::not_null<ECS::iSystemRegister *> _system_register,
                  gsl::not_null<ECS::iComponentRegister *> _component_register);

    //iSceneInstance
    bool ReadyForActivation() const override { return active_fences.empty(); }

    //iStepableObject
    void DoStep(double time_delta) override;

private:
    iAsyncLoader *const async_loader;

    std::recursive_mutex mutex;
    std::unordered_set<std::string> active_fences;

    std::vector<std::unique_ptr<ECS::iSystem>> all_systems;

    boost::container::static_vector<iStepableObject *, ECS::kMaxStepableSystems> stepable_systems;
    ECS::ComponentArray component_array;

    //returns true if NO fence is set
    bool SetFenceState(std::string name, bool state);
    void LoadSceneContent(gsl::not_null<ECS::iSystemRegister *> _system_register, std::string &_file_data);
};

} // namespace MoonGlare::SceneManager