#pragma once

#include <typeinfo>
#include <string>

#include "Configuration.h"

namespace MoonGlare::Component {

using ComponentClassId = uint16_t;

class BaseComponentInfo {
public:
    static ComponentClassId GetComponentClassesCount() { return idAlloc; }
    using Destructor = void(void*);

    struct ComponentClassInfo {
        size_t byteSize;
        //ApiInitializer(*m_ApiInit)(ApiInitializer) = nullptr;
        const BaseComponentInfo *infoPtr = nullptr;
        Destructor *destructor;
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
    template<class T>
    static ComponentClassId AllocateComponentClass();
private:
    static ComponentClassId AllocateID() { return idAlloc++; }
    static ComponentClassId idAlloc;
    using ComponentClassesTypeTable = std::array<ComponentClassInfo, Configuration::MaxComponentTypes>;
    static ComponentClassesTypeTable componentClassesTypeInfo;
};

template<class T>
struct ComponentInfo : public BaseComponentInfo {
    static ComponentClassId GetClassID() { return classId; }
    //static_assert(std::is_pod<T>::value, "Component must be pod type!");

    const std::type_info &GetTypeInfo() const override { return typeid(T); }
private:
    static ComponentClassId classId;
};

template<typename T>
ComponentClassId ComponentInfo<T>::classId = BaseComponentInfo::AllocateComponentClass<T>();

template<typename T>
ComponentClassId ComponentClassIdValue = ComponentInfo<T>::GetClassID();

template<class T>
static ComponentClassId BaseComponentInfo::AllocateComponentClass() {
    auto id = AllocateID();
    static const ComponentInfo<T> t;
    assert(id < Configuration::MaxComponentTypes);
    componentClassesTypeInfo[id] = {
        sizeof(T),
        //&T::RegisterLuaApi,
        &t,
        &BaseComponentInfo::DestructorFunc<T>,
#ifdef DEBUG
        std::is_pod<T>::value
#endif
    };
    return id;
}

template<typename T>
static const BaseComponentInfo::ComponentClassInfo& BaseComponentInfo::GetComponentTypeInfo() {
    return BaseComponentInfo::GetComponentTypeInfo(ComponentInfo<T>::GetClassID());
}

}
