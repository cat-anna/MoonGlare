#pragma once

#include <DockWindowInfo.h>
#include <Notifications.h>

#include <ToolBase/Modules/ChangesManager.h>
#include <ToolBase/UserQuestions.h>

#include <DataModels/StructureEditingModel.h>

namespace Ui { class TreeViewDialog; }

namespace MoonGlare {
namespace Editor {

class TreeViewDialog
	: public QDialog
	, public iSettingsUser {
	Q_OBJECT
public:
    TreeViewDialog(QWidget *parent, const std::string settingsName);
	~TreeViewDialog();
    QTreeView* GetTreeView();
protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

private:
	std::unique_ptr<Ui::TreeViewDialog> ui;
protected slots:
};

} //namespace Editor
} //namespace MoonGlare
