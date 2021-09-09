
#include "scene_manager/default_scene_factory.hpp"
#include "scene_instance.hpp"

namespace MoonGlare::SceneManager {

std::unique_ptr<iSceneInstance> DefaultSceneFactory::CreateSceneInstance(std::string scene_name,
                                                                         FileResourceId res_id) {
    return std::make_unique<SceneInstance>(std::move(scene_name), res_id, 0, async_loader,
                                           component_register, prefab_manager, rendering_device);
}

} // namespace MoonGlare::SceneManager
