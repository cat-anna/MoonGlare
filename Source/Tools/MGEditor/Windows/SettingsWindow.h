#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <DockWindowInfo.h>
#include <Notifications.h>
#include <ToolBase/Modules/ChangesManager.h>
#include <ToolBase/UserQuestions.h>

#include <DataModels/StructureEditingModel.h>

namespace Ui { class SettingsWindow; }

namespace MoonGlare {
namespace Editor {

class SettingsWindow
	: public QDialog
	, public UserQuestions
	, public MoonGlare::Module::iSettingsUser {
	Q_OBJECT
public:
	SettingsWindow(SharedModuleManager modmgr, QWidget *parent = 0);
	~SettingsWindow();
protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

    SharedModuleManager GetModuleManager() {
        return moduleManager;
    }
private:
	std::unique_ptr<Ui::SettingsWindow> m_Ui;
    SharedModuleManager moduleManager;
protected slots:
};

} //namespace Editor
} //namespace MoonGlare

#endif // SETTINGSWINDOW_H
