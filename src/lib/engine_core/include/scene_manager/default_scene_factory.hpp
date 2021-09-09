#pragma once

#include "async_loader.hpp"
#include "ecs/component_interface.hpp"
#include "prefab_manager_interface.hpp"
#include "renderer/facade.hpp"
#include "scenes_manager_interface.hpp"

namespace MoonGlare::SceneManager {

class DefaultSceneFactory : public iSceneInstanceFactory {
public:
    DefaultSceneFactory(gsl::not_null<iAsyncLoader *> _async_loader,
                        gsl::not_null<ECS::iComponentRegister *> _component_register,
                        gsl::not_null<iPrefabManager *> _prefab_manager,
                        gsl::not_null<Renderer::iRenderingDeviceFacade *> _rendering_device)
        : async_loader(_async_loader), component_register(_component_register),
          prefab_manager(_prefab_manager), rendering_device(_rendering_device) {}
    ~DefaultSceneFactory() override = default;

    std::unique_ptr<iSceneInstance> CreateSceneInstance(std::string scene_name,
                                                        FileResourceId res_id) override;

private:
    iAsyncLoader *const async_loader;
    ECS::iComponentRegister *const component_register;
    iPrefabManager *const prefab_manager;
    Renderer::iRenderingDeviceFacade *const rendering_device;
};

} // namespace MoonGlare::SceneManager
