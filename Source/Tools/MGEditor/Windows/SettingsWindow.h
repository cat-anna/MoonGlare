#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <DockWindowInfo.h>
#include <Notifications.h>
#include <qtUtils.h>
#include <ChangesManager.h>

#include <DataModels/StructureEditingModel.h>

namespace Ui { class SettingsWindow; }

namespace MoonGlare {
namespace Editor {

class SettingsWindow
	: public QDialog
	, public QtShared::UserQuestions
	, public iSettingsUser {
	Q_OBJECT
public:
	SettingsWindow(QWidget *parent = 0);
	~SettingsWindow();
protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
private:
	std::unique_ptr<Ui::SettingsWindow> m_Ui;
protected slots:
};

} //namespace Editor
} //namespace MoonGlare

#endif // SETTINGSWINDOW_H
