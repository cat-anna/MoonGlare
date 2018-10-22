#pragma once

#include <typeinfo>
#include <string>

#include <boost/tti/has_static_member_data.hpp>
#include <boost/tti/has_member_function.hpp>

#include <Foundation/Scripts/ApiInit.h>
#include "iSubsystem.h"

#include "Configuration.h"
#include "Entity.h"

namespace MoonGlare::Component {
namespace detail {

template <typename T, typename Tuple>
struct HasFlag;

template <typename T, typename... Us>
struct HasFlag<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

}

enum class SystemClassId : uint16_t { Invalid = 0xFFFF, };

class BaseSystemInfo {
public:
    static SystemClassId GetUsedSystemTypes() { return static_cast<SystemClassId>(idAlloc); }

    struct SystemClassInfo {
        SystemClassId id;
        const BaseSystemInfo *infoPtr = nullptr;
        const char* systemName = nullptr;
        //bool required = false;
    };

    template<typename T>
    static const SystemClassInfo& GetSystemTypeInfo();
    static const SystemClassInfo& GetSystemTypeInfo(SystemClassId id) { return GetSystemClassesTypeInfo()[static_cast<size_t>(id)]; }
    static std::optional<SystemClassId> GetClassByName(const std::string &cname);

    static void Dump(std::ostream &output);

    template<typename FUNC>
    static void ForEachSystem(FUNC && func) {
        for (size_t i = 0; i < (size_t)GetUsedSystemTypes(); ++i)
            func((SystemClassId)i, GetSystemTypeInfo((SystemClassId)i));
    }

    virtual const std::type_info &GetTypeInfo() const = 0;
    virtual UniqueSubsystem MakeInstance(iSubsystemManager* owner) const = 0;
protected:
    template<class T>
    static SystemClassId AllocateSystemClass();
    using SystemClassesTypeTable = std::array<SystemClassInfo, Configuration::MaxComponentTypes>;
    static SystemClassesTypeTable& GetSystemClassesTypeInfo();
    static void SetNameMapping(SystemClassId ccid, std::string name);
private:
    static SystemClassId AllocateId() { return static_cast<SystemClassId>(idAlloc++); }
    static std::underlying_type_t<SystemClassId> idAlloc;
};

template<class T>
struct SystemInfo : public BaseSystemInfo {
    static SystemClassId GetClassId() { return classId; }

    const std::type_info &GetTypeInfo() const override { return typeid(T); }

    UniqueSubsystem MakeInstance(iSubsystemManager* owner) const override {
        return std::make_unique<T>(owner);
    }
    //using Flags = iSubsystem::Flags;
private:
    static const SystemClassId classId;
};

template<typename T>
const SystemClassId SystemInfo<T>::classId = BaseSystemInfo::AllocateSystemClass<T>();

template<typename T>
SystemClassId SystemClassIdValue = SystemInfo<T>::GetClassId();

template<class T>
static SystemClassId BaseSystemInfo::AllocateSystemClass() {
    auto id = AllocateId();
    static const SystemInfo<T> t;
    assert((uint32_t)id < Configuration::MaxSystemTypes);
    GetSystemClassesTypeInfo()[(size_t)id] = {
        id,
        &t,
        T::SystemName,
        //T::Required,
    };
    SetNameMapping(id, T::SystemName);
    return id;
}

template<typename T>
static const BaseSystemInfo::SystemClassInfo& BaseSystemInfo::GetSystemTypeInfo() {
    return BaseSystemInfo::GetSystemTypeInfo(SystemInfo<T>::GetClassID());
}

}
