#pragma once

#include "ecs/entity_manager_interface.hpp"
#include "ecs/system_interface.hpp"
#include "stepable_interface.hpp"
#include <boost/container/static_vector.hpp>
#include <gsl/gsl>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

namespace MoonGlare::SceneManager {

class iPrefabManager {
public:
    virtual ~iPrefabManager() = default;

    struct LoadedSystems {
        boost::container::static_vector<iStepableObject *, ECS::kMaxStepableSystems> stepable_systems;
        std::vector<std::unique_ptr<ECS::iSystem>> systems;
    };

    virtual LoadedSystems LoadSystemConfiguration(const ECS::SystemCreateInfo &data,
                                                  const nlohmann::json &config_node) = 0;

    virtual void LoadRootEntity(gsl::not_null<ECS::iEntityManager *> entity_manager,
                                const nlohmann::json &child_node) = 0;

    // virtual iCompo
};

#ifdef WANTS_GTEST_MOCKS
struct PrefabManagerMock : public iPrefabManager {
    MOCK_METHOD2(LoadSystemConfiguration, LoadedSystems(const ECS::SystemCreateInfo &, const nlohmann::json &));
    MOCK_METHOD2(LoadRootEntity, void(gsl::not_null<ECS::iEntityManager *>, const nlohmann::json &));
};
#endif

} // namespace MoonGlare::SceneManager
