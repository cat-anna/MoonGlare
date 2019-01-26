#pragma once

#include <DockWindowInfo.h>
#include <Notifications.h>
#include <ToolBase/UserQuestions.h>
#include <iEditor.h>
#include <iFileProcessor.h>

#include <MiscIfs.h>
#include <ToolBase/Module.h>
#include <ToolBase/Modules/iSettingsUser.h>
#include <ToolBase/interfaces/ActionBarSink.h>
#include <ToolBase/interfaces/MainWindowTabs.h>

namespace Ui { class MainWindow; }

namespace MoonGlare {
namespace Editor {

namespace Module {
	class DataModule;
}
class FileSystem;
using SharedFileSystem = std::shared_ptr<FileSystem>;

class MainWindow
	: public QMainWindow
	, public iModule
	, public UserQuestions
	, public MoonGlare::Module::iSettingsUser
	, public QtShared::MainWindowProvider
	, public QtShared::QtWindowProvider<MainWindow> 
    , public iActionBarSink
    , public iMainWindowTabsCtl
    {
	Q_OBJECT
public:
	MainWindow(SharedModuleManager modmgr);
	~MainWindow();
	static MainWindow* Get();
protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
	
	QMainWindow *GetMainWindowWidget() override { return this; }
	bool PostInit() override;
    bool Finalize() override;
private:
	std::unique_ptr<Ui::MainWindow> m_Ui;
    QTimer refreshTimer;

    QLabel *jobProcessorStatus = nullptr;

	std::shared_ptr<QtShared::EditorProvider> m_EditorProvider;
    QtShared::SharedJobProcessor jobProcessor;

	std::unordered_map<std::string, QtShared::SharedBaseDockWindowModule> m_Editors;

	Module::SharedDataModule m_DataModule;
	void NewModule(const std::string& MasterFile);
	void OpenModule(const std::string& MasterFile);
	void CloseModule();

    void AddAction(std::string id, ActionVariant action, std::weak_ptr<iActionProvider> provider) final;
    void RemoveProvider(std::weak_ptr<iActionProvider> provider) final;
    struct ActionInfo {
        ActionVariant action;
        std::weak_ptr<iActionProvider> provider;
        QAction* GetActionPtr() const {
            return std::visit([this](auto item) ->QAction* {
                if constexpr (std::is_same_v<QAction*, decltype(item)>)
                    return item;
                else
                    return nullptr;
            }, action);
        }
    };
    std::unordered_map<std::string, ActionInfo> actionBarItems;

//iMainWindowTabsCtl
    QWidget* GetTabParentWidget() const override;
    void AddTab(const std::string &id, std::shared_ptr<iTabViewBase> tabWidget) override;
    bool TabExists(const std::string &id) const override;
    void ActivateTabs(const std::string &id) override;
    void TabCloseRequested(int index);
    std::unordered_map<std::string, std::shared_ptr<iTabViewBase>> openedTabs;
public slots:
    void RefreshStatus();
protected slots:
	void NewModuleAction();
	void OpenModuleAction();
	void CloseModuleAction();
	void CloseEditorAction();
    void BuildModuleAction();
    void PackModuleAction();
};

} //namespace Editor
} //namespace MoonGlare

