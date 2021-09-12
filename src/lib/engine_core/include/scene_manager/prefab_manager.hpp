#pragma once

#include "ecs/ecs_register.hpp"
#include "ecs/entity_manager_interface.hpp"
#include "prefab_manager_interface.hpp"
#include "readonly_file_system.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace MoonGlare::SceneManager {

class PrefabManager final : public iPrefabManager {
public:
    PrefabManager(gsl::not_null<iReadOnlyFileSystem *> _filesystem,
                  gsl::not_null<ECS::iSystemRegister *> _system_register,
                  gsl::not_null<ECS::iComponentRegister *> _component_register,
                  gsl::not_null<iRuntimeResourceLoader *> _runtime_resource_loader);
    ~PrefabManager() override;

    LoadedSystems LoadSystemConfiguration(const ECS::SystemCreateInfo &data,
                                          const nlohmann::json &config_node) override;

    void LoadRootEntity(gsl::not_null<ECS::iEntityManager *> entity_manager,
                        const nlohmann::json &child_node) override;

    void ClearCache();
    void PrintCache() const;

private:
    iReadOnlyFileSystem *const filesystem;
    ECS::iSystemRegister *const system_register;
    ECS::iComponentRegister *const component_register;
    iRuntimeResourceLoader *const runtime_resource_loader;

    mutable std::mutex cache_mutex;
    std::unordered_map<FileResourceId, std::unique_ptr<nlohmann::json>> json_cache;

    struct ImportTask;
    struct ComponentImport;

    void Load(ImportTask &task);

    void ProcessEntityNode(const nlohmann::json &config_node, int entity_index, ImportTask &task);

    nlohmann::json &LoadJson(FileResourceId res_id);

#if 0
    std::unordered_map<std::string, std::unique_ptr<ImportData>> prefabCache;
#endif
};

} // namespace MoonGlare::SceneManager
