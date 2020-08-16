#pragma once

#include "dock_window.hpp"
#include <QIcon>
#include <QKeySequence>
#include <QMainWindow>
#include <QObject>
#include <runtime_modules.h>
#include <runtime_modules/settings_provider.hpp>

namespace MoonGlare::Tools {

struct BaseDockWindowModuleRuntimeData {
    bool visible = false;
};

class BaseDockWindowModule : public QObject,
                             public iModule,
                             public RuntineModules::iSettingsProvider {
    Q_OBJECT

public:
    BaseDockWindowModule(SharedModuleManager modmgr);
    virtual ~BaseDockWindowModule();

    bool Finalize() override;

    const QString &GetIconResName() const { return icon_name; }
    const QString &GetDisplayName() const { return display_name; }
    const QString &GetShortcut() const { return short_cut; }
    bool IsMainMenu() const { return main_menu; }

    QIcon GetIcon() const { return QIcon(GetIconResName()); }
    QKeySequence GetKeySequence() const { return QKeySequence(GetShortcut()); }

    std::shared_ptr<DockWindow> GetInstance();
    void ReleaseInstance();
    void SetParent(QMainWindow *p) { parent = p; }
public slots:
    void Show();

protected:
    QMainWindow *parent{nullptr};
    QString icon_name;
    QString display_name;
    QString short_cut;
    std::shared_ptr<DockWindow> instance;
    bool main_menu = true;
    BaseDockWindowModuleRuntimeData runtime_data;
    void SetIconResName(QString v) { icon_name.swap(v); }
    void SetDisplayName(QString v) { display_name.swap(v); }
    void SetShortcut(QString v) { short_cut.swap(v); }
    void SetMainMenu(bool v) { main_menu = v; }

    virtual std::shared_ptr<DockWindow> CreateInstance() = 0;

    void DoSaveSettings(nlohmann::json &json) const override;
    void DoLoadSettings(const nlohmann::json &json) override;
protected slots:
    void WindowClosed(DockWindow *Sender);
};

template <typename DOCK>
class DockWindowModule : public BaseDockWindowModule {
public:
    DockWindowModule(SharedModuleManager modmgr) : BaseDockWindowModule(std::move(modmgr)) {}

protected:
    virtual std::shared_ptr<DockWindow> CreateInstance() {
        return std::make_shared<DOCK>(parent, GetModuleManager());
    }
};

using SharedBaseDockWindowModule = std::shared_ptr<BaseDockWindowModule>;

} // namespace MoonGlare::Tools
