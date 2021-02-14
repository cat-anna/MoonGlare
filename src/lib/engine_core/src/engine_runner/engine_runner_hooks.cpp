#include "engine_runner/engine_runner_hooks.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Runner {

void EngineRunnerHooksHost::Clear() {
    installed_hooks.clear();
}

void EngineRunnerHooksHost::AfterDataModulesLoad() {
    AddLog(Debug, "Running hook AfterDataModulesLoad");
    for (const auto &item : installed_hooks) {
        std::visit([](auto &ptr) { ptr->AfterDataModulesLoad(); }, item);
    }
}

void EngineRunnerHooksHost::InstallHook(iEngineRunnerHooks *ptr) {
    installed_hooks.emplace_back(HookVariantType(ptr));
}

void EngineRunnerHooksHost::InstallHook(std::unique_ptr<iEngineRunnerHooks> ptr) {
    installed_hooks.emplace_back(HookVariantType(std::move(ptr)));
}

void EngineRunnerHooksHost::InsertInterfaceHook(const std::type_info &info, std::function<void(std::any)> functor) {
    interface_ready_hooks[std::type_index(info)].emplace_back(std::move(functor));
}

void EngineRunnerHooksHost::RunInterfaceHook(const std::type_info &info, std::any any_ptr) {
    AddLog(Debug, fmt::format("Running hook for {} interface", info.name()));
    auto functor_vector = std::move(interface_ready_hooks[std::type_index(info)]);
    interface_ready_hooks.erase(std::type_index(info));
    for (auto &item : functor_vector) {
        item(any_ptr);
    }
}

void EngineRunnerHooksHost::AssertAllInterfaceHooksExecuted() const {
    if (interface_ready_hooks.empty()) {
        AddLog(Debug, "All interface hooks were executed");
        return;
    }

    for (auto &[index, vec] : interface_ready_hooks) {
        AddLog(Error, fmt::format("There are {} hooks pending for interface {}", vec.size(), index.name()));
    }
    throw std::runtime_error("Not all iterface ready hooks was executed!");
}

} // namespace MoonGlare::Runner