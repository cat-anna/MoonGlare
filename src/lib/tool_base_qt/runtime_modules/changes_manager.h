#pragma once

#include <qobject.h>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class ChangesManager;
struct iChangeContainer;

class ChangesManager : public QObject, public iModule {
    Q_OBJECT;

public:
    ChangesManager(SharedModuleManager modmgr);
    virtual ~ChangesManager();

    void SetModifiedState(iChangeContainer *sender, bool value);

    struct State {
        bool m_Modified;
    };
    using StateMap = std::unordered_map<iChangeContainer *, State>;
    const StateMap &GetStateMap() { return m_State; }
    void SaveAll();

signals:
    void Changed(iChangeContainer *sender, bool state);

protected:
    StateMap m_State;
};

struct iChangeContainer {
    iChangeContainer(SharedModuleManager sharedModuleManager) : sharedModuleManager(std::move(sharedModuleManager)) {}
    virtual ~iChangeContainer() { SetModifiedState(false); };
    virtual bool SaveChanges() = 0;
    // virtual bool CanDropChanges() const { return false; }
    // virtual bool DropChanges() { return false; }

    virtual std::string GetInfoLine() const { return ""; }
    virtual const std::string &GetName() const { return m_Name; }

    bool IsChanged() const { return m_Changed; }

    void SetChangesParent(iChangeContainer *Parent) { m_Parent = Parent; }
    iChangeContainer *GetParent() const { return m_Parent; }

    void InsertChangesChild(iChangeContainer *child, std::string childName) {
        child->SetChangesParent(this);
        child->SetChangesName(std::move(childName));
        m_Children.push_back(child);
    }
    void SetChangesName(std::string Name) { m_Name.swap(Name); }

    void SetModuleManager(SharedModuleManager smm) { sharedModuleManager.swap(smm); }

protected:
    virtual void SetModifiedState(bool value) {
        if (value == m_Changed)
            return;
        m_Changed = value;
        if (value) {
            if (m_Parent) {
                m_Parent->SetModifiedState(value);
            }
        } else {
        }
        assert(sharedModuleManager);
        auto cm = sharedModuleManager->QueryModule<ChangesManager>();
        if (cm)
            cm->SetModifiedState(this, value);
    }

private:
    iChangeContainer *m_Parent = nullptr;
    bool m_Changed = false;
    std::string m_Name;
    SharedModuleManager sharedModuleManager;
    std::vector<iChangeContainer *> m_Children;
};

} // namespace MoonGlare::Tools::RuntineModules
