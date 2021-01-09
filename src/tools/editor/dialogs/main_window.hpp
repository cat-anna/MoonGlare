#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <main_window_tabs.hpp>
#include <qevent.h>
#include <runtime_modules.h>
#include <runtime_modules/app_config.h>
#include <runtime_modules/widget_settings_provider.hpp>
#include <user_questions.hpp>

// #include <Notifications.h>
// #include <iEditor.h>
// #include <iFileProcessor.h>
// #include <ToolBase/Module.h>
// #include <ToolBase/interfaces/ActionBarSink.h>

namespace Ui {
class MainWindow;
}

namespace MoonGlare::Tools::Editor::Dialogs {

struct MainWindowSettings {
    std::vector<std::string> recent_modules;
    bool load_last_module = true;
};

class MainWindow : public QMainWindow,
                   public iModule,
                   public UserQuestions,
                   public RuntineModules::iWidgetSettingsProvider,
                   //    public QtShared::MainWindowProvider,
                   //    public QtShared::QtWindowProvider<MainWindow>,
                   //    public iActionBarSink,
                   public iMainWindowTabsControl {
    Q_OBJECT
public:
    MainWindow(SharedModuleManager modmgr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

    void DoSaveSettings(nlohmann::json &json) const override;
    void DoLoadSettings(const nlohmann::json &json) override;

    // QMainWindow *GetMainWindowWidget() override { return this; }
    bool PostInit() override;
    bool Finalize() override;

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QTimer refresh_timer;

    QLabel *job_processor_status = nullptr;

    MainWindowSettings settings;
    std::weak_ptr<RuntineModules::AppConfig> app_config;

    // std::shared_ptr<QtShared::EditorProvider> m_EditorProvider;
    // QtShared::SharedJobProcessor jobProcessor;

    // std::unordered_map<std::string, QtShared::SharedBaseDockWindowModule> m_Editors;

    void OpenModule(const std::string &root_directory);
    //     void CloseModule();

    //     void AddAction(std::string id, ActionVariant action, std::weak_ptr<iActionProvider> provider) final;
    //     void RemoveProvider(std::weak_ptr<iActionProvider> provider) final;
    //     struct ActionInfo {
    //         ActionVariant action;
    //         std::weak_ptr<iActionProvider> provider;
    //         QAction *GetActionPtr() const {
    //             return std::visit(
    //                 [this](auto item) -> QAction * {
    //                     if constexpr (std::is_same_v<QAction *, decltype(item)>)
    //                         return item;
    //                     else
    //                         return nullptr;
    //                 },
    //                 action);
    //         }
    //     };
    //     std::unordered_map<std::string, ActionInfo> actionBarItems;

    // iMainWindowTabsControl
    QWidget *GetTabParentWidget() const override;
    void AddTab(const std::string &id, std::shared_ptr<iTabViewBase> tabWidget) override;
    std::shared_ptr<iTabViewBase> GetTab(const std::string &id) const override;
    void SetTabTitle(iTabViewBase *tab_view, const std::string &title) override;
    void SetTabIcon(iTabViewBase *tab_view, QIcon icon) override;
    bool TabExists(const std::string &id) const override;
    void ActivateTab(iTabViewBase *tab_view) override;
    void ActivateTab(const std::string &id) override;
    void TabCloseRequested(int index);
    std::unordered_map<std::string, std::shared_ptr<iTabViewBase>> opened_tabs;
public slots:
    void RefreshStatus();
protected slots:
    void NewModuleAction();
    void OpenModuleAction();
    void CloseModuleAction();
    void CloseEditorAction();

    //     void BuildModuleAction();
    //     void PackModuleAction();
};

} // namespace MoonGlare::Tools::Editor::Dialogs
