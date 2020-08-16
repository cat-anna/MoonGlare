#pragma once

#include <dynamic_class_register.h>
#include <interface_map.h>
#include <orbit_logger.h>

namespace MoonGlare::Tools {

class ModuleManager;
using SharedModuleManager = std::shared_ptr<ModuleManager>;
using WeakModuleManager = std::weak_ptr<ModuleManager>;

class iModule : public std::enable_shared_from_this<iModule> {
public:
    using SharedModuleManager = SharedModuleManager;

    iModule(SharedModuleManager modmgr) : m_ModuleManager(std::move(modmgr)) {}
    virtual ~iModule() {}

    template <typename T>
    std::shared_ptr<T> cast() {
        auto casted = std::dynamic_pointer_cast<T>(shared_from_this());
        if (casted) {
            return casted;
        }
        try {
            auto queried = QueryInterface(typeid(T));
            if (queried.has_value()) {
                return std::any_cast<std::shared_ptr<T>>(queried);
            }
        } catch (const std::exception &e) {
            AddLogf(Error, "QueryInterface failed with exception: %s", e.what());
        }
        return nullptr;
    }

    virtual const std::string &GetModuleName() { return m_Alias; }

    virtual bool Initialize() { return true; }
    virtual bool PostInit() { return true; }
    virtual bool Finalize() { return true; }

    virtual std::any QueryInterface(const std::type_info &info) { return {}; }

    template <typename T>
    void SetAlias(T &&t) {
        m_Alias = std::forward<T>(t);
    }

    SharedModuleManager GetModuleManager() const { return m_ModuleManager.lock(); }

private:
    std::string m_Alias;
    WeakModuleManager m_ModuleManager;
};

using ModuleClassRegister = DynamicClassRegister<iModule, SharedModuleManager>;
using SharedModule = std::shared_ptr<iModule>;
using WeakModule = std::weak_ptr<iModule>;
using SharedModuleTable = std::vector<SharedModule>;

template <typename T>
struct ModuleInterfacePair {
    SharedModule module;
    std::shared_ptr<T> interface;
};

class ModuleManager : public std::enable_shared_from_this<ModuleManager> {
public:
    virtual ~ModuleManager() {}
    static SharedModuleManager
    CreateModuleManager(std::unordered_map<std::string, std::string> AppConfig = {});
    bool Initialize();
    bool Finalize();

    struct NotFoundException {};
    struct MultipleInstancesFoundException {};

    template <typename T>
    std::vector<ModuleInterfacePair<T>> QueryInterfaces() {
        std::vector<ModuleInterfacePair<T>> ret;
        ret.reserve(m_Modules.size());
        for (auto &ptr : m_Modules) {
            auto t1 = std::dynamic_pointer_cast<T>(ptr);
            if (t1) {
                ret.emplace_back(ModuleInterfacePair<T>{ptr, std::move(t1)});
                continue;
            }
            try {
                auto any = ptr->QueryInterface(typeid(T));
                if (any.has_value()) {
                    auto t2 = std::any_cast<std::shared_ptr<T>>(any);
                    if (t2) {
                        ret.emplace_back(ModuleInterfacePair<T>{ptr, std::move(t2)});
                    }
                }
            } catch (const std::bad_any_cast &e) {
                AddLogf(Error,
                        "Bad any object returned from module %s "
                        "QueryInterface [%s]",
                        ptr->GetModuleName().c_str(), e.what());
            }
        }
        auto it = m_CustomInterfaces.find(std::type_index(typeid(T)));
        if (it != m_CustomInterfaces.end())
            ret.emplace_back(
                ModuleInterfacePair<T>{nullptr, std::any_cast<std::shared_ptr<T>>(it->second)});
        return std::move(ret);
    }

    template <typename T>
    std::shared_ptr<T> QueryModule() {
        auto ifs = QueryInterfaces<T>();
        if (ifs.size() > 1)
            throw MultipleInstancesFoundException();
        if (ifs.empty())
            throw NotFoundException();
        return ifs.front().interface;
    }

    template <typename T>
    void AddInterface(std::shared_ptr<T> intf) {
        m_CustomInterfaces[std::type_index(typeid(T))] = intf;
    }
    InterfaceMap &GetInterfaceMap() { return interfaceMap; }

    template <typename T, typename O>
    void RegisterInterface(O *o) {
        auto optr = o->shared_from_this();
        auto tptr = std::dynamic_pointer_cast<T>(optr);
        if (tptr)
            interfaceMap.SetSharedInterface(tptr);
        else
            __debugbreak();
    }

    template <typename T, typename F>
    void ForEachInterface(F f) {
        // TODO: can be optimized
        for (auto &item : QueryInterfaces<T>()) {
            f(item.interface, item.module);
        }
    }

protected:
    ModuleManager();
    InterfaceMap interfaceMap;
    std::vector<SharedModule> m_Modules;
    std::unordered_map<std::type_index, std::any> m_CustomInterfaces;
};
using SharedModuleManager = std::shared_ptr<ModuleManager>;

} // namespace MoonGlare::Tools
