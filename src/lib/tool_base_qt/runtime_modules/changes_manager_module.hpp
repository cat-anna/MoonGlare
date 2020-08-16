#pragma once

#include "change_container.hpp"
#include <qobject.h>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class ChangesManagerModule : public iChangesManager, public iModule {
    Q_OBJECT;

public:
    ChangesManagerModule(SharedModuleManager modmgr);
    virtual ~ChangesManagerModule();

    void SetModifiedState(std::shared_ptr<iChangeContainer> sender, bool value) override;

    StateContainer GetStateMap() override { return state; }
    void SaveAll() override;
    bool IsChanged() const { return !state.empty(); }
    void DropChanges() override;

protected:
    StateContainer state;
};

} // namespace MoonGlare::Tools::RuntineModules
