#include "prefab_manager.hpp"
// #include "Component/ComponentRegister.h"
// #include "Core/Component/SubsystemManager.h"
// #include <Foundation/Module/iDebugContext.h>
// #include <boost/algorithm/string.hpp>
// #include <nfMoonGlare.h>
// #include <pch.h>

namespace MoonGlare::Core {
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
        static int dumpid = 0;
        auto fname = fmt::format("logs/{}.entity", dumpid++);

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

struct PrefabManager::ImportTask {
    std::unique_ptr<ImportData> customData;

    ImportData *importData;
    Component::iSubsystemManager *manager;
    Component::Entity parent;
    std::string name;

    std::vector<Component::Entity> spawnEntity;
    std::vector<Component::Entity> localRelationsBuffer;
};

//-------------------------------------------------------------------------------------------------

PrefabManager::PrefabManager(InterfaceMap &ifaceMap) {
    ifaceMap.GetObject(fileSystem);
    ifaceMap.GetObject(entityManager);


    auto dbgCtx = ifaceMap.GetInterface<Module::iDebugContext>();
    if (dbgCtx) {
        dbgCtx->AddDebugCommand("ClearPrefabCache", [this] () { ClearCache(); });
        dbgCtx->AddDebugCommand("PrintPrefabCache", [this] () { PrintCache(); });
    }
}

PrefabManager::~PrefabManager() {
#ifdef DEBUG_DUMP
    PrintCache();
#endif //  DEBUG_DUMP
}

void PrefabManager::ClearCache() {
    prefabCache.clear();
    xmlCache.clear();
}

//-------------------------------------------------------------------------------------------------

Component::Entity PrefabManager::Spawn(Component::iSubsystemManager *manager, Component::Entity parent, const std::string &uri, const std::string &name) {
    ImportTask task;
    task.importData = Import(uri);

    task.name = name;
    task.parent = parent;
    task.manager = manager;

    return Spawn(task);
}

Component::Entity PrefabManager::Spawn(Component::iSubsystemManager *manager, Component::Entity parent, const pugi::xml_node node, const std::string &name, const std::string &srcName) {
    ImportTask task;
    task.customData = Import(node);
    task.importData = task.customData.get();

    task.importData->srcName = srcName;
    task.name = name;
    task.parent = parent;
    task.manager = manager;

    return Spawn(task);
}

//-------------------------------------------------------------------------------------------------

Component::Entity PrefabManager::Spawn(ImportTask &task) {
    assert(task.importData);

    auto &data = *task.importData;
    task.spawnEntity.resize(data.entities.size());

    for (size_t index = 0; index < data.entities.size(); ++index) {
        auto &ei = data.entities[index];
        if (!ei.enabled)
            continue;

        Entity thisParent;
        if (ei.parentIndex >= 0)
            thisParent = task.spawnEntity[ei.parentIndex];
        else
            thisParent = task.parent;

        auto &eout = task.spawnEntity[index];
        if (!entityManager->Allocate(thisParent,eout, ei.name)) {
            AddLogf(Error, "Failed to allocate entity!");
            //return {};
        }
        if (thisParent.GetIndex() > eout.GetIndex()) {
            int i = 0;
        }
    }

    for (auto &c : data.componentSpawnOrder)
        for (auto &ci : *c.componentImport)
            SpawnComponent(task, ci);

    return task.spawnEntity[0];
}

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

std::unique_ptr<PrefabManager::ImportData> PrefabManager::Import(const pugi::xml_node node) {
    std::unique_ptr<ImportData> data = std::make_unique<ImportData>();
    Import(*data, node);
    return std::move(data);
}

pugi::xml_node PrefabManager::GetPrefabXml(const std::string &uri) {
    auto &cache = xmlCache[uri];
    if (!cache) {
        if (!fileSystem->OpenXML(cache, uri)) {
            AddLogf(Error, "Failed to open uri: %s", uri.c_str());
            return {};
        }
        AddLogf(Performance, "Loaded xml: %s", uri.c_str());
    } else {
        AddLogf(Performance, "Prefab xml cache hit: %s", uri.c_str());
    }
    return *cache;
}

//-------------------------------------------------------------------------------------------------

void PrefabManager::Import(ImportData &importData, pugi::xml_node node) {
    Import(importData, node, -1);
    importData.Preprocess();
}

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
        case "Entity"_Hash32:
        {
            EntityImport ei;
            ei.parentIndex = entityIndex;
            ei.enabled = it.attribute("Enabled").as_bool(true) && parentEnabled;
            ei.name = it.attribute("Name").as_string();

            int32_t index = static_cast<int32_t>(data.entities.size());
            if (ei.parentIndex >= 0)
                data.entities[ei.parentIndex].children[ei.name] = index;

            data.entities.emplace_back(std::move(ei));

            auto pattern = it.attribute("Pattern").as_string(nullptr);
            if (pattern) {
                auto childNode = GetPrefabXml(pattern);
                auto &ei = data.entities.back();
                if (ei.name.empty())
                    ei.name = childNode.attribute("Name").as_string();
                Import(data, childNode.first_child(), index);
            }
            Import(data, it, index);
            continue;
        }
        default:
            AddLogf(Warning, "Unknown node: %s", nodename);
            continue;
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

//-------------------------------------------------------------------------------------------------

void PrefabManager::PrintCache() const {
    std::stringstream ss;
    ss << "PrefabManager cache:\n";

    ss << "Cached xml:\n";
    for (auto&[uri, _] : xmlCache)
        ss << "\t" << uri << "\n";
    ss << "Cached prefabs:\n";
    for (auto&[uri, _] : prefabCache)
        ss << "\t" << uri << "\n";

    AddLog(Resources, ss.str());
}
#endif
} // namespace MoonGlare::Core
