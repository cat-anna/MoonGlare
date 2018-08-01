#pragma once

#include <typeinfo>
#include <string>

#include "Configuration.h"

#include <EngineBase/Scripts/ApiInit.h>

namespace MoonGlare::Component {

using ComponentClassId = uint16_t;

class BaseComponentInfo {
public:
    static ComponentClassId GetUsedComponentTypes() { return idAlloc; }
    using ComponentFunc = void(void*);
    using ComponentScriptPush = int(void*, lua_State *lua);

    struct ComponentClassInfo {
        ComponentClassId id;
        size_t byteSize;
        Scripts::ApiInitFunc apiInitFunc;
        const BaseComponentInfo *infoPtr = nullptr;
        ComponentScriptPush *scriptPush = nullptr;
        ComponentFunc *destructor = nullptr;
        ComponentFunc *constructor = nullptr;
        const char* componentName = nullptr;
#ifdef DEBUG
        bool pod = false;
#endif
    };

    template<typename T>
    static const ComponentClassInfo& GetComponentTypeInfo();
    static const ComponentClassInfo& GetComponentTypeInfo(ComponentClassId id) { return componentClassesTypeInfo[id]; }

    static void Dump(std::ostream &output);

    virtual const std::type_info &GetTypeInfo() const = 0;
protected:
    template<typename T> 
    static void DestructorFunc(void* ptr) { reinterpret_cast<T*>(ptr)->~T(); }
    template<typename T>
    static void ConstructorFunc(void* ptr) { new (ptr) T (); }
    template<typename T>
    static int ScriptPush(void* ptr, lua_State *lua) { 
        luabridge::push<T*>(lua, reinterpret_cast<T*>(ptr)); 
        return 1;
    }

    template<class T>
    static ComponentClassId AllocateComponentClass();
private:
    static ComponentClassId AllocateId() { return idAlloc++; }
    static ComponentClassId idAlloc;
    using ComponentClassesTypeTable = std::array<ComponentClassInfo, Configuration::MaxComponentTypes>;
    static ComponentClassesTypeTable componentClassesTypeInfo;
};

template<class T>
struct ComponentInfo : public BaseComponentInfo {
    static ComponentClassId GetClassId() { return classId; }
    //static_assert(std::is_pod<T>::value, "Component must be pod type!");

    const std::type_info &GetTypeInfo() const override { return typeid(T); }
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
    assert(id < Configuration::MaxComponentTypes);
    componentClassesTypeInfo[id] = {
        id,
        sizeof(T),
        Scripts::GetApiInitFunc<T>(),
        &t,
        &BaseComponentInfo::ScriptPush<T>,
        &BaseComponentInfo::DestructorFunc<T>,
        &BaseComponentInfo::ConstructorFunc<T>,
        T::ComponentName,
#ifdef DEBUG
        std::is_pod<T>::value,
#endif
    };
    return id;
}

template<typename T>
static const BaseComponentInfo::ComponentClassInfo& BaseComponentInfo::GetComponentTypeInfo() {
    return BaseComponentInfo::GetComponentTypeInfo(ComponentInfo<T>::GetClassID());
}

}
