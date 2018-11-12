#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Scripts/ApiInit.h>

#include "../Component/SubsystemManager.h"

#include <Core/PrefabManager.h>

namespace MoonGlare::Core::Scripts::Component {
using namespace MoonGlare::Core::Component;
using namespace MoonGlare::Component;
using namespace MoonGlare::Scripts;

class ScriptComponent;

struct GameObject {
    static constexpr char TagFieldName[] = "Tag";

    Entity owner;
    int Tag;

    SubsystemManager *myWorld;
    ScriptComponent *scriptComponent;
    EntityManager *entityManager;
    PrefabManager *prefabManager;


    int CreateComponent(lua_State *lua);
    int GetComponent(lua_State * lua);

    int LoadObject(lua_State *lua);
    void Destroy();

    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
protected:
    void SetName(const char *name);
    const char* GetName() const;
};

struct GameObjectTable {
    void Clear() { gameObjectTable.fill({}); }
    GameObject* GetObject(Entity e) {
        auto &ptr = gameObjectTable[e.GetIndex()];
        if (ptr.owner != e)
            return nullptr;
        return &ptr;
    }
    void Release(Entity e) {
        auto &ptr = gameObjectTable[e.GetIndex()];
        ptr = {};
    }
    GameObject* Set(Entity e, SubsystemManager *world, ScriptComponent *scriptComponent, EntityManager *entityManager, PrefabManager *prefabManager) {
        auto &ptr = gameObjectTable[e.GetIndex()];
        ptr = {};
        ptr.owner = e;
        ptr.entityManager = entityManager;
        ptr.prefabManager = prefabManager;
        ptr.scriptComponent = scriptComponent;
        ptr.myWorld = world;
        return &ptr;
    }
protected:
    Component::Configuration::EntityArray<GameObject> gameObjectTable;
};

static constexpr size_t GameObjectSize = sizeof(GameObject);
static constexpr size_t GameObjectTableSize = sizeof(GameObjectTable)/1024;

}
