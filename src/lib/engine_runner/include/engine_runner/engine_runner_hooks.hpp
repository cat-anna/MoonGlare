
#pragma once

#include "interface_hooks.hpp"
#include <any>
#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include <vector>

namespace MoonGlare::Runner {

class iEngineRunnerHooks {
public:
    virtual ~iEngineRunnerHooks() = default;

    virtual void AfterDataModulesLoad() = 0;
};

class iEngineRunnerHooksHost : public iEngineRunnerHooks, public iInterfaceHooks {
public:
    virtual ~iEngineRunnerHooksHost() = default;

    virtual void InstallHook(iEngineRunnerHooks *) = 0;
    virtual void InstallHook(std::unique_ptr<iEngineRunnerHooks>) = 0;
};

class EngineRunnerHooksHost : public iEngineRunnerHooksHost {
public:
    void Clear();

    //iEngineRunnerHooks
    void AfterDataModulesLoad() override;

    //iEngineRunnerHooksHost
    void InstallHook(iEngineRunnerHooks *ptr) override;
    void InstallHook(std::unique_ptr<iEngineRunnerHooks> ptr) override;

    void AssertAllInterfaceHooksExecuted() const;

protected:
    //iInterfaceHooks
    void RunInterfaceHook(const std::type_info &info, std::any any_ptr) override;
    void InsertInterfaceHook(const std::type_info &info, std::function<void(std::any)> functor);

private:
    using HookVariantType = std::variant<iEngineRunnerHooks *, std::unique_ptr<iEngineRunnerHooks>>;
    std::vector<HookVariantType> installed_hooks;

    // std::unordered_map<std::type_index,std::any> known_interfaces; //TODO:?
    std::unordered_map<std::type_index, std::vector<std::function<void(std::any)>>> interface_ready_hooks;
};

} // namespace MoonGlare::Runner