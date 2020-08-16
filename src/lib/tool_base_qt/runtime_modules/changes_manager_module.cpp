#include "changes_manager_module.hpp"
#include <orbit_logger.h>

namespace MoonGlare::Tools::RuntineModules {

ChangesManagerModule::ChangesManagerModule(SharedModuleManager modmgr) : iModule(modmgr) {
}

ChangesManagerModule::~ChangesManagerModule() {
}

void ChangesManagerModule::SetModifiedState(std::shared_ptr<iChangeContainer> sender, bool value) {
    if (!sender) {
        AddLog(Error, "Attempt to mark as changed a null change container");
        return;
    }
    auto it = state.find(sender);
    if (it == state.end()) {
        if (value) {
            state[sender] = {value};
        }
    } else {
        if (value) {
            it->second.modified = value;
        } else {
            state.erase(it);
        }
    }

    emit Changed(sender, value);
}

void ChangesManagerModule::SaveAll() {
    // TODO: move this to job procesor
    while (!state.empty()) {
        auto it = state.begin();
        auto locked = it->first;
        if (locked) {
            locked->SaveChanges();
        } else {
            state.erase(it);
        }
    }
}

void ChangesManagerModule::DropChanges() {
    state.clear();
}

} // namespace MoonGlare::Tools::RuntineModules
