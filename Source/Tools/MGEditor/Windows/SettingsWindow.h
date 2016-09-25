#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <DockWindowInfo.h>
#include <Notifications.h>
#include <qtUtils.h>
#include <ChangesManager.h>

#include <TypeEditor/CustomType.h>
#include <TypeEditor/Structure.h>

namespace Ui { class SettingsWindow; }

namespace MoonGlare {
namespace Editor {

class SettingsWindow
	: public QDialog
	, public QtShared::UserQuestions
	, public QtShared::iSettingsUser {
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
	std::unique_ptr<QStandardItemModel> m_Model;
	TypeEditor::UniqueStructure m_SettingsStructure;
public slots:
	void Refresh();	
protected slots:
};

} //namespace Editor
} //namespace MoonGlare

#endif // SETTINGSWINDOW_H
