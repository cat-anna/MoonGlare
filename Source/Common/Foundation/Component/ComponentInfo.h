#pragma once

#include <typeinfo>
#include <string>

#include <boost/tti/has_static_member_data.hpp>
#include <boost/tti/has_member_function.hpp>

#include <Foundation/Scripts/ApiInit.h>

#include "Configuration.h"
#include "Entity.h"

namespace MoonGlare::Component {

namespace detail {
BOOST_TTI_HAS_STATIC_MEMBER_DATA(ComponentLimit);
BOOST_TTI_HAS_MEMBER_FUNCTION(Load);
}

class ComponentArray;
struct ComponentReader;

enum class ComponentClassId : uint16_t { Invalid = 0xFFFF, };

class BaseComponentInfo {
public:
    static ComponentClassId GetUsedComponentTypes() { return static_cast<ComponentClassId>(idAlloc); }
    using ComponentFunc = void(void*);
    using ComponentBiFunc = void(void*, void*);
    using ComponentScriptPush = int(ComponentArray *carray, Entity owner, lua_State *lua);
    using ComponentLoadFunc = bool(void*, ComponentReader &reader, Entity owner);

    struct ComponentClassInfo {
        ComponentClassId id;
        size_t byteSize;
        const BaseComponentInfo *infoPtr = nullptr;
        ComponentScriptPush *scriptPush = nullptr;
        Scripts::ApiInitFunc apiInitFunc = nullptr;
        ComponentFunc *destructor = nullptr;
        ComponentFunc *constructor = nullptr;
        ComponentBiFunc *swap = nullptr;
        ComponentLoadFunc *load = nullptr;
        const char* componentName = nullptr;
        bool isTrivial = false;
    };

    template<typename T>
    static const ComponentClassInfo& GetComponentTypeInfo();
    static const ComponentClassInfo& GetComponentTypeInfo(ComponentClassId id) { return GetComponentClassesTypeInfo()[static_cast<size_t>(id)]; }

    static std::optional<ComponentClassId> GetClassByName(const std::string &cname);

    static void Dump(std::ostream &output);

    template<typename FUNC>
    static void ForEachComponent(FUNC && func) {
        for (size_t i = 0; i < (size_t)GetUsedComponentTypes(); ++i)
            func((ComponentClassId)i, GetComponentTypeInfo((ComponentClassId)i));
    }

    virtual const std::type_info &GetTypeInfo() const = 0;
    virtual uint32_t GetDefaultCapacity() const = 0;
protected:
    template<typename T> 
    static void DestructorFunc(void* ptr) { reinterpret_cast<T*>(ptr)->~T(); }
    template<typename T>
    static void ConstructorFunc(void* ptr) { new (ptr) T (); }
    template<typename T>
    static void SwapFunc(void* a, void* b) {
        std::swap(*reinterpret_cast<T*>(a), *reinterpret_cast<T*>(b));
    }
    template<typename T>
    static bool LoadFunc(void* ptr, ComponentReader &reader, Entity owner) {
        if constexpr (detail::has_member_function_Load<T, bool(T::*)(ComponentReader &reader, Entity owner)>::value)
            return reinterpret_cast<T*>(ptr)->Load(reader, owner);
        else
        return true;
    }
    template<typename T, typename WRAP>
    static int ScriptPush(ComponentArray *carray, Entity owner, lua_State *lua) {
        WRAP lw{ };
        lw.owner = owner;
        lw.componentArray = carray;
        luabridge::push<WRAP>(lua, lw);
        return 1;
    }

    template<class T>
    static ComponentClassId AllocateComponentClass();
    using ComponentClassesTypeTable = std::array<ComponentClassInfo, Configuration::MaxComponentTypes>;
    static ComponentClassesTypeTable& GetComponentClassesTypeInfo();
    static void SetNameMapping(ComponentClassId ccid, std::string name);
private:
    static ComponentClassId AllocateId() { return static_cast<ComponentClassId>(idAlloc++); }
    static std::underlying_type_t<ComponentClassId> idAlloc;
};

template<class T>
struct ComponentInfo : public BaseComponentInfo {
    static ComponentClassId GetClassId() { return classId; }
    //static_assert(std::is_pod<T>::value, "Component must be pod type!");

    const std::type_info &GetTypeInfo() const override { return typeid(T); }
    uint32_t GetDefaultCapacity() const override {
        if constexpr (detail::has_static_member_data_ComponentLimit<T, const uint32_t>::value) {
            return static_cast<uint32_t>(T::ComponentLimit);
        } else {
            return Configuration::MaxComponentInstances;
        }
    }

    template<typename WRAP>
    static void SetScriptWrapper() {
        GetComponentClassesTypeInfo()[(size_t)GetClassId()].scriptPush = &ScriptPush<T, WRAP>;
        GetComponentClassesTypeInfo()[(size_t)GetClassId()].apiInitFunc = Scripts::GetApiInitFunc<WRAP>();
    }
private:
    static const ComponentClassId classId;
};

template<typename T>
const ComponentClassId ComponentInfo<T>::classId = BaseComponentInfo::AllocateComponentClass<T>();

template<typename T>
ComponentClassId ComponentClassIdValue = ComponentInfo<T>::GetClassId();

template<class T>
static ComponentClassId BaseComponentInfo::AllocateComponentClass() {
    auto id = AllocateId();
    static const ComponentInfo<T> t;
    assert((uint32_t)id < Configuration::MaxComponentTypes);
    GetComponentClassesTypeInfo()[(size_t)id] = {
        id,
        sizeof(T),
        &t,
        nullptr, //&BaseComponentInfo::ScriptPush<T>,
        nullptr,
        &BaseComponentInfo::DestructorFunc<T>,
        &BaseComponentInfo::ConstructorFunc<T>,
        &BaseComponentInfo::SwapFunc<T>,
        &BaseComponentInfo::LoadFunc<T>,
        T::ComponentName,
        std::is_trivial<T>::value,
    };
    SetNameMapping(id, T::ComponentName);
    return id;
}

template<typename T>
static const BaseComponentInfo::ComponentClassInfo& BaseComponentInfo::GetComponentTypeInfo() {
    return BaseComponentInfo::GetComponentTypeInfo(ComponentInfo<T>::GetClassID());
}

}
