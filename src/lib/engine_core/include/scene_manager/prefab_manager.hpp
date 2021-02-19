#pragma once

#include "prefab_manager_interface.hpp"

namespace MoonGlare::SceneManager {

class PrefabManager final : public iPrefabManager {
public:
    virtual ~PrefabManager() = default;

    LoadedSystems LoadSystemConfiguration(const ECS::SystemCreateInfo &data,
                                          const nlohmann::json &config_node) override {
        return {};
    }

    void LoadRootEntity(gsl::not_null<ECS::iEntityManager *> entity_manager,
                        const nlohmann::json &child_node) override{};

    // virtual iCompo
};

/*

for (auto &item : all_systems) {
    auto ptr = dynamic_cast<iStepableObject *>(item.get());
    if (ptr) {
        stepable_systems.push_back(ptr);
    }
}
AddLog(Performance,
        fmt::format("Got {} stepable systems in scene {}", stepable_systems.size(), GetSceneName()));

*/

} // namespace MoonGlare::SceneManager

#if 0

#pragma once

#include "scene_manager/prefab_manager_interface.hpp"
#include <string>
#include <unordered_map>
#include <vector>
// #include <Foundation/Component/Entity.h>
// #include <Foundation/Component/iSubsystem.h>
// #include <Foundation/iFileSystem.h>
// #include <interface_map.h>
// #include <Foundation/Component/EntityManager.h>

namespace MoonGlare::SeneManager {

#if 0
// TODO: this should be thread-safe
class PrefabManager final {
  public:
    PrefabManager(InterfaceMap &ifaceMap);
    ~PrefabManager();

    void ClearCache();
    void PrintCache() const;

    void LoadPrefab(const std::string &uri) { Import(uri); }

    Component::Entity Spawn(Component::iSubsystemManager *Manager,
                            Component::Entity parent, const std::string &uri,
                            const std::string &name = "");
    Component::Entity Spawn(Component::iSubsystemManager *Manager,
                            Component::Entity parent, const pugi::xml_node node,
                            const std::string &name = "",
                            const std::string &srcName = "");

  private:
    struct ImportData;
    struct ImportTask;
    struct EntityImport;
    struct ComponentImport;

    iFileSystem *fileSystem = nullptr;
    Component::EntityManager *entityManager = nullptr;

    std::unordered_map<std::string, XMLFile> xmlCache;
    std::unordered_map<std::string, std::unique_ptr<ImportData>> prefabCache;

    ImportData *Import(const std::string &uri);
    std::unique_ptr<ImportData> Import(const pugi::xml_node node);

    void Import(ImportData &importData, pugi::xml_node node);
    void Import(ImportData &importData, pugi::xml_node node,
                int32_t parentIndex);

    Component::Entity Spawn(ImportTask &task);
    void SpawnComponent(ImportTask &task, const ComponentImport &ci);

    pugi::xml_node GetPrefabXml(const std::string &uri);
};

#endif
} // namespace MoonGlare::SeneManager

#endif
