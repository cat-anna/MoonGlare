#pragma once

#include <build_configuration.hpp>

namespace MoonGlare::SceneManager {

constexpr auto kLoadingSceneName = "loading_scene";

// constexpr auto kResourceFenceName = "system.resources";
constexpr auto kSceneFenceSystemsLoadPending = "scene.systems.pending";
constexpr auto kSceneFenceEntitiesPending = "scene.entities.pending";

constexpr auto kSceneEntityConfig = "entities";
constexpr auto kSceneSystemsConfig = "systems";

} // namespace MoonGlare::SceneManager
