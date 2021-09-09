#include "scene_manager/prefab_manager.hpp"
#include "component/json_serialization.hpp"
#include "core/stepable_interface.hpp"
#include "ecs/component_info.hpp"
#include <array>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::SceneManager {

namespace {
//
};

//----------------------------------------------------------------------------------

struct PrefabManager::ComponentImport {
    ECS::Entity entity = 0;
    bool active = true;
    const nlohmann::json *import_data = nullptr;
};

struct PrefabManager::ImportTask {
    ECS::iEntityManager *entity_manager;
    ECS::iComponentArray *component_array;
    ECS::iComponentRegister *component_register;

    using ProcessEntry = std::tuple<PrefabManager *, const nlohmann::json &, int, ImportTask &>;
    std::list<ProcessEntry> entries_to_process;

    std::vector<ECS::Entity> all_entities;
    std::vector<int> entity_parent;

    std::array<std::vector<ComponentImport>, Component::kMaxComponents> components;

    void ProcessAllEntries() {
        while (!entries_to_process.empty()) {
            ImportTask::ProcessEntry item = std::move(entries_to_process.front());
            entries_to_process.pop_front();
            std::apply(&PrefabManager::ProcessEntityNode, item);
        }
    }

    void SpawnComponents() {
        for (auto c_id = 0; c_id < components.size(); ++c_id) {
            if (components[c_id].empty()) {
                continue;
            }

            auto *c_info = component_register->GetComponentsInfo(c_id);
            AddLog(Performance, fmt::format("Importing {} components of class {}:{}",
                                            components[c_id].size(), c_id, c_info->GetName()));

            auto &io_ops = c_info->GetIoOps();

            if (io_ops.construct_from_json == nullptr) {
                AddLog(Error, fmt::format("Component of class {}:{} is not importable from json!",
                                          c_id, c_info->GetName()));
                continue;
            }

            for (const auto &c_import : components[c_id]) {
                InstantiateComponent(c_import, c_id, io_ops.construct_from_json);
            }
        }
    }

    void InstantiateComponent(const ComponentImport &c_import, Component::ComponentId c_id,
                              ECS::BaseComponentInfo::ComponentConstJsonFunc json_op) {

        ECS::iComponentArray::IndexType index;
        if (!entity_manager->GetEntityIndex(c_import.entity, index)) {
            AddLog(Error, "Failed to get index of entity!");
            return;
        }

        if (!c_import.import_data) {
            AddLog(Error, "No import data");
            return;
        }

        auto *mem = component_array->CreateComponent(index, c_id, false);
        json_op(mem, *c_import.import_data);
        component_array->SetComponentActive(index, c_id, c_import.active);
    }
};

//----------------------------------------------------------------------------------

PrefabManager::PrefabManager(gsl::not_null<iReadOnlyFileSystem *> _filesystem,
                             gsl::not_null<ECS::iSystemRegister *> _system_register,
                             gsl::not_null<ECS::iComponentRegister *> _component_register)
    : filesystem(_filesystem), system_register(_system_register),
      component_register(_component_register) {
}

PrefabManager::~PrefabManager() {
#ifdef DEBUG_DUMP
    PrintCache();
#endif
}

void PrefabManager::ClearCache() {
    std::lock_guard<std::mutex> guard(cache_mutex);
    json_cache.clear();
}

void PrefabManager::PrintCache() const {
    std::lock_guard<std::mutex> guard(cache_mutex);
    AddLog(Resources, fmt::format("PrefabManager cache (json={})", json_cache.size()));
    for (auto &[res_id, _] : json_cache) {
        AddLog(Resources,
               fmt::format("  json {:016x} -> {}", res_id, filesystem->GetNameOfResource(res_id)));
    }
}

nlohmann::json &PrefabManager::LoadJson(FileResourceId res_id) {
    std::lock_guard<std::mutex> guard(cache_mutex);
    auto it = json_cache.find(res_id);

    if (it != json_cache.end()) {
        AddLog(Performance, fmt::format("Prefab json cache hit {:x}", res_id));
        return *it->second.get();
    }

    AddLog(Performance, fmt::format("Prefab json cache miss {:x}", res_id));

    std::string file_data;
    if (!filesystem->ReadFileByResourceId(res_id, file_data)) {
        AddLog(Error, fmt::format("Failed to read imported child {:x}", res_id));
        throw std::runtime_error("Failed to read imported child");
    }

    auto json = std::make_unique<nlohmann::json>(nlohmann::json::parse(file_data));
    auto json_ptr = json.get();
    json_cache[res_id] = std::move(json);

    return *json_ptr;
}

//----------------------------------------------------------------------------------

PrefabManager::LoadedSystems
PrefabManager::LoadSystemConfiguration(const ECS::SystemCreateInfo &data,
                                       const nlohmann::json &config_node) {

    LoadedSystems ls;
    ls.systems = system_register->LoadSystemConfiguration(data, config_node);

    for (auto &item : ls.systems) {
        if (item->GetSystemInfo().stepable) {
            ls.stepable_systems.push_back(item.get());
        }
    }

    return ls;
}

void PrefabManager::LoadRootEntity(gsl::not_null<ECS::iEntityManager *> entity_manager,
                                   const nlohmann::json &child_node) {

    ImportTask task;
    task.component_register = component_register;
    task.entity_manager = entity_manager;
    task.component_array = entity_manager->GetComponentArray();

    task.all_entities = {entity_manager->GetRootEntity()};
    task.entity_parent = {-1};

    task.entries_to_process.emplace_back(this, child_node, 0, task);

    Load(task);
}

//----------------------------------------------------------------------------------

void PrefabManager::Load(ImportTask &task) {
    try {
        task.ProcessAllEntries();
        task.SpawnComponents();
    } catch (const std::exception &e) {
        AddLog(Error, fmt::format("Failed to load entity: '{}'", e.what()));
    }
}

void PrefabManager::ProcessEntityNode(const nlohmann::json &config_node, int entity_index,
                                      ImportTask &task) {
    Component::JsonEntity je;
    config_node.get_to(je);
    if (!je.enabled) {
        return;
    }

    if (je.import_id.has_value()) {
        ProcessEntityNode(LoadJson(je.import_id.value()), entity_index, task);
        return;
    }

    auto local_root = task.all_entities[entity_index];

    if (je.children != nullptr && je.children->is_array()) {
        auto children_count = je.children->size();

        task.all_entities.reserve(children_count);
        task.entity_parent.reserve(children_count);

        //   int child_index = 0;
        for (auto &child_node : *je.children) {
            auto child = task.entity_manager->NewEntity(local_root);
            int child_index = static_cast<int>(task.entity_parent.size());
            task.entity_parent.emplace_back(entity_index);
            task.all_entities.emplace_back(child);
            task.entries_to_process.emplace_back(this, child_node, child_index, task);
        }
    }

    if (je.components != nullptr && je.components->is_array()) {
        for (auto &component_node : *je.components) {
            Component::JsonComponent jc;
            component_node.get_to(jc);
            if (jc.enabled) {
                task.components[jc.component_id].emplace_back(ComponentImport{
                    .entity = local_root,
                    .active = jc.active,
                    .import_data = jc.data,
                });
            }
        }
    }
}

//----------------------------------------------------------------------------------

#if 0

struct PrefabManager::EntityImport {
    int32_t parentIndex = -1;
    std::string name;
    bool enabled = true;
    std::unordered_map<std::string, int32_t> children;
};

struct PrefabManager::ComponentImport {
    int32_t entityIndex = 1;
    bool enabled = true;
    bool active = true;
    pugi::xml_node xmlNode;
    std::vector<std::string> localRelationNames;
    std::vector<int32_t> localRelationIndex;
};

struct PrefabManager::ImportData {
    std::vector<EntityImport> entities;
    using ComponentImportVector = std::vector<ComponentImport>;
    std::unordered_map<Component::SubSystemId, ComponentImportVector> components;

    struct ComponentOrderInfo {
        int orderTag;
        const ComponentImportVector* componentImport;
        bool operator<(const ComponentOrderInfo &oth) const { return orderTag < oth.orderTag; }
    };
    std::vector<ComponentOrderInfo> componentSpawnOrder;

    std::string srcName;

    int32_t FindSibling(int32_t startIndex, const std::string &Name) {
        if (startIndex < 0)
            return startIndex;
        auto &e = entities[startIndex];
        return FindChild(e.parentIndex < 0 ? startIndex : e.parentIndex, Name);
    }

    int32_t FindChild(int32_t startIndex, const std::string &Name) {
        if (startIndex < 0)
            return startIndex;

        auto &e = entities[startIndex];
        if (e.name == Name)
            return startIndex;

        for (auto&[n, idx] : e.children) {
            if (auto r = FindChild(idx, Name); r >= 0)
                return r;
        }
        return -1;
    }

    void Preprocess() {
        static const std::unordered_map<SubSystemId, int> ComponentOrder = {
            {SubSystemId::Transform, 0},
            {SubSystemId::RectTransform, 1},
            {SubSystemId::Body, 2},
            {SubSystemId::Script, 0xFFFF},
        };

        componentSpawnOrder.reserve(components.size());

        for (auto &c : components) {
            std::sort(c.second.begin(), c.second.end(), [](const ComponentImport &c1, const ComponentImport &c2) {
                return c1.entityIndex < c2.entityIndex;
            });

            for (auto &ci : c.second) {
                ci.localRelationIndex.reserve(ci.localRelationNames.size());
                for (auto& item : ci.localRelationNames) {
                    auto rel = FindSibling(ci.entityIndex, item);
                    ci.localRelationIndex.emplace_back(rel);
                }
            }

            auto orderIt = ComponentOrder.find(c.first);
            int orderTag = orderIt != ComponentOrder.end() ? orderIt->second : (int)c.first + 0x0100;

            componentSpawnOrder.emplace_back(ComponentOrderInfo{ orderTag, &c.second });
        }

        std::sort(componentSpawnOrder.begin(), componentSpawnOrder.end());

#ifdef DEBUG_DUMP
        Dump();
#endif
    }

    void Dump() {
        static int dump_id = 0;
        auto fname = fmt::format("logs/{}.entity", dump_id++);

        std::ofstream of(fname, std::ios::out);

        if (!srcName.empty())
            of << "SOURCE: " << srcName << "\n\n";

        of << fmt::format("Entities: {}\n", entities.size());
        size_t index = 0;
        for (auto&it : entities) {
            of << fmt::format("\tIndex:{:3} ParentIndex:{:3} Enabled:{} Name:{}\n", index, it.parentIndex, it.enabled, it.name);
            ++index;
        }
        of << "\n";
        of << fmt::format("Components: {}\n", components.size());
        for (auto &it : components) {
            std::string Name;
            if (it.first > SubSystemId::CoreBegin)
                Name = Component::ComponentRegister::GetComponentInfo(it.first)->m_Name;
            else
                Name = Component::BaseComponentInfo::GetComponentTypeInfo(static_cast<Component::ComponentClassId>(it.first)).componentName;

            of << fmt::format("\tComponent:{}  Name:{}\n", (int)it.first, Name);
            for (auto &c : it.second) {
                std::string rels;
                for (size_t i = 0; i < c.localRelationNames.size(); ++i) {
                    if (!rels.empty())
                        rels += ",";
                    rels += c.localRelationNames[i] + "=" + std::to_string(c.localRelationIndex[i]);
                }
                of << fmt::format("\t\tParentIndex:{:3}  Enabled:{}  Active:{}  Relations:{} \n", c.entityIndex, c.enabled, c.active, rels);
            }
        }

        of.close();
    }
};

//-------------------------------------------------------------------------------------------------

PrefabManager::ImportData* PrefabManager::Import(const std::string &uri) {
    auto &data = prefabCache[uri];
    if (!data) {
        data = std::make_unique<ImportData>();
        data->srcName = uri;
        auto node = GetPrefabXml(uri);
        Import(*data, node);
        AddLogf(Performance, "Loaded prefab: %s", uri.c_str());
    } else {
        AddLogf(Performance, "Prefab cache hit: %s", uri.c_str());
    }
    return data.get();
}

//-------------------------------------------------------------------------------------------------

void PrefabManager::Import(ImportData &data, pugi::xml_node node, int32_t entityIndex) {
    bool parentEnabled = entityIndex >= 0 ? data.entities[entityIndex].enabled : true;

    if (entityIndex >= 0) {
        auto &ei = data.entities[entityIndex];
        if (ei.name.empty())
            ei.name = node.attribute("Name").as_string("");

        ei.enabled = node.attribute("Enabled").as_bool(true) && parentEnabled;
    }

    for (auto it = node.first_child(); it; it = it.next_sibling()) {
        const char *nodename = it.name();
        auto hash = Space::Utils::MakeHash32(nodename);

        switch (hash) {
        case "Component"_Hash32:
        {
            ComponentImport ci;
            ci.xmlNode = it;
            ci.enabled = it.attribute("Enabled").as_bool(true) && parentEnabled;
            ci.active = it.attribute("Active").as_bool(true);
            std::string localRels = it.child("LocalRelations").text().as_string("");
            if (!localRels.empty())
                boost::split(ci.localRelationNames, localRels, boost::is_any_of(","));

            SubSystemId cid = SubSystemId::Invalid;
            if (!Component::ComponentRegister::ExtractCIDFromXML(it, cid)) {
                AddLogf(Warning, "Unknown component!");
                continue;
            }
            assert(entityIndex >= 0);
            ci.entityIndex = entityIndex;

            auto &c = data.components[cid];
            auto found = std::find_if(c.begin(), c.end(), [entityIndex](const ComponentImport& import) { return import.entityIndex == entityIndex; });
            if (found != c.cend()) {
                *found = std::move(ci);
            } else {
                c.emplace_back(std::move(ci));
            }
            continue;
        }
        }
    }
}

//-------------------------------------------------------------------------------------------------

void PrefabManager::SpawnComponent(ImportTask &task, const ComponentImport &ci) {
    if (!ci.enabled)
        return;

    SubSystemId cid = SubSystemId::Invalid;

    if (!Component::ComponentRegister::ExtractCIDFromXML(ci.xmlNode, cid)) {
        AddLogf(Warning, "Unknown component!");
        return;
    }

    auto &data = *task.importData;

    Entity parent = {};
    Entity owner = {};
    if (ci.entityIndex >= 0) {
        auto &e = data.entities[ci.entityIndex];
        owner = task.spawnEntity[ci.entityIndex];
        if (e.parentIndex >= 0)
            parent = task.spawnEntity[e.parentIndex];
        else
            parent = task.parent;
    } else {
        __debugbreak();
        //owner = Owner;
    }

    auto gen = owner.GetGeneration();
    if (gen > 1) {
        int i = 0;
    }

    task.localRelationsBuffer.clear();
    for (auto rel : ci.localRelationIndex) {
        if (rel >= 0)
            task.localRelationsBuffer.push_back(task.spawnEntity[rel]);
        else
            task.localRelationsBuffer.push_back({});
    }

    MoonGlare::Component::ComponentReader reader{ task.manager, ci.xmlNode };
    if (!task.localRelationsBuffer.empty()) {
        reader.localRelations = &task.localRelationsBuffer[0];
        reader.localRelationsCount = task.localRelationsBuffer.size();
    }
    if (cid < SubSystemId::CoreBegin) {
        auto cci = static_cast<Component::ComponentClassId>(cid);
        bool r = task.manager->GetComponentArray().Load(owner, cci, reader);
        task.manager->GetComponentArray().SetActive(owner, ci.active, cci);
        return;
    }

    auto &ssm = dynamic_cast<Core::Component::SubsystemManager&>(*task.manager);

    auto c = ssm.GetComponent(cid);
    if (!c) {
        //AddLogf(Warning, "No such component: %d", cid);
        return;
    }

    if (!c->Load(reader, parent, owner)) {
        AddLogf(Error, "Failure during loading component! cid:%d class: %s", cid, typeid(*c).name());
        return;
    }
}

#endif

} // namespace MoonGlare::SceneManager
