#pragma once

#include <QIcon>
#include <QObject>
#include <memory>
#include <runtime_modules.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace MoonGlare::Tools {

struct iChangeContainer;

class iChangesManager : public QObject {
    Q_OBJECT;

public:
    virtual void SetModifiedState(std::shared_ptr<iChangeContainer> sender, bool value) = 0;

    struct State {
        bool modified;
    };

    using StateContainer = std::unordered_map<std::shared_ptr<iChangeContainer>, State>;
    virtual StateContainer GetStateMap() = 0;
    virtual void SaveAll() = 0;
    virtual bool IsChanged() const = 0;
    virtual void DropChanges() = 0;

signals:
    void Changed(std::shared_ptr<iChangeContainer> sender, bool state);

protected:
    virtual ~iChangesManager() = default;
};

struct iChangeContainer {
    iChangeContainer(SharedModuleManager shared_module_manager,
                     std::weak_ptr<iChangeContainer> parent = {})
        : parent(std::move(parent)) {
        assert(shared_module_manager);
        changes_manager = shared_module_manager->QueryModule<iChangesManager>();
    }

    virtual bool SaveChanges() = 0;
    virtual bool DropChanges() { return false; }
    virtual bool DropChangesPossible() { return false; }

    virtual std::string GetInfoLine() const { return ""; }
    virtual QIcon GetInfoIcon() const { return {}; }
    virtual std::string_view GetName() const { return name; }
    std::weak_ptr<iChangeContainer> GetParent() const { return parent; }

    bool IsChanged() const { return changed; }

    void InsertChangesChild(std::shared_ptr<iChangeContainer> child) { children.push_back(child); }
    void SetChangesName(std::string Name) { name.swap(Name); }

protected:
    virtual ~iChangeContainer() = default;

    virtual void SetModifiedState(bool value) {
        if (value == changed) {
            return;
        }
        changed = value;
        if (value) {
            auto locked_parent = parent.lock();
            if (locked_parent) {
                locked_parent->SetModifiedState(value);
            }
        } else {
        }
        auto locked_changes_manager = changes_manager.lock();
        if (locked_changes_manager) {
            locked_changes_manager->SetModifiedState(GetSelfPointer(), value);
        }
    }

    virtual std::shared_ptr<iChangeContainer> GetSelfPointer() = 0;

private:
    std::weak_ptr<iChangeContainer> parent;
    bool changed = false;
    std::string name;
    std::weak_ptr<iChangesManager> changes_manager;
    std::vector<std::weak_ptr<iChangeContainer>> children;
};

template <typename GluedClass>
class iTypedChangeContainer : public iChangeContainer {
public:
    iTypedChangeContainer(SharedModuleManager shared_module_manager,
                          std::weak_ptr<iChangeContainer> parent = {})
        : iChangeContainer(std::move(shared_module_manager), std::move(parent)) {}

protected:
    ~iTypedChangeContainer() override = default;

    std::shared_ptr<iChangeContainer> GetSelfPointer() override {
        return std::dynamic_pointer_cast<iChangeContainer>(
            dynamic_cast<GluedClass *>(this)->shared_from_this());
    }
};

Q_DECLARE_METATYPE(std::shared_ptr<iChangeContainer>);
Q_DECLARE_METATYPE(std::weak_ptr<iChangeContainer>);

} // namespace MoonGlare::Tools
