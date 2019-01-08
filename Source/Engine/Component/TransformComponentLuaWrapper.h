#pragma once

namespace MoonGlare::Component {

/*@ [ComponentReference/TransformComponentLuaWrapper] Transform component
    Component is responsible for 3d space transformation @*/   
struct TransformComponent::LuaWrapper {
    TransformComponent *component;
    Core::Scripts::Component::ScriptComponent *scriptComponent;
    Entity owner;
    mutable ComponentIndex index;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
        index = component->GetComponentIndex(owner);
        //}
        if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference deleted Transform component! ");
        }
    }

    math::vec3 GetPosition() const { Check(); return emath::MathCast<math::fvec3>(component->GetPosition(index)); }
    void SetPosition(math::vec3 pos) { Check(); component->SetPosition(index, emath::MathCast<emath::fvec3>(pos)); }
    math::vec4 GetRotation() const { Check(); return emath::MathCast<math::fvec4>(component->GetRotation(index)); }
    void SetRotation(math::vec4 rot) { Check(); component->SetRotation(index, emath::MathCast<emath::Quaternion>(rot)); }
    math::vec3 GetScale() const { Check(); return emath::MathCast<math::fvec3>(component->GetScale(index)); }
    void SetScale(math::vec3 s) { Check(); component->SetScale(index, emath::MathCast<emath::fvec3>(s)); }

    int FindChildByName(lua_State *lua) {
        Check();
        std::string_view name = lua_tostring(lua, 2);
        if (name.empty())
            return 0;

        auto *em = component->entityManager;
        for (auto childIndex : component->values.TraverseTree(index)) {
            Entity e = component->values.owner[childIndex];
            std::string n;
            if (em->GetEntityName(e, n) && name == n) {
                return scriptComponent->GetGameObject(lua, e);
            }
        }
        return 0;
    }
    int GetParent(lua_State *lua) {
        Check();
        auto parentIndex = component->values.parentIndex[index];
        if (parentIndex == component->values.InvalidIndex)
            return 0;
        auto parentEntity = component->values.owner[parentIndex];
        return scriptComponent->GetGameObject(lua, parentEntity);
    }
    int GetFirstChild(lua_State *lua) {
        Check();
        auto firstChild = component->values.firstChild[index];
        if (firstChild == component->values.InvalidIndex)
            return 0;
        auto childEntity = component->values.owner[firstChild];
        return scriptComponent->GetGameObject(lua, childEntity);
    }
    int GetNextSibling(lua_State *lua) {
        Check();
        auto nextSibling = component->values.nextSibling[index];
        if (nextSibling == component->values.InvalidIndex)
            return 0;
        auto siblingEntity = component->values.owner[nextSibling];
        return scriptComponent->GetGameObject(lua, siblingEntity);
    }

    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
        return root
            .beginClass<LuaWrapper>("TransformComponent")
            .addCFunction("GetParent", &LuaWrapper::GetParent)
            .addCFunction("GetFirstChild", &LuaWrapper::GetFirstChild)
            .addCFunction("GetNextSibling", &LuaWrapper::GetNextSibling)
            .addCFunction("FindChildByName", &LuaWrapper::FindChildByName)

            .addProperty("Position", &LuaWrapper::GetPosition, &LuaWrapper::SetPosition)
            .addProperty("Rotation", &LuaWrapper::GetRotation, &LuaWrapper::SetRotation)
            .addProperty("Scale", &LuaWrapper::GetScale, &LuaWrapper::SetScale)
            .endClass()
            ;
    }
};

}
