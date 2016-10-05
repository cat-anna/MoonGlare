#include PCH_HEADER
#include <qobject.h>
#include "EditorSettings.h"
#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <DockWindow.h>
#include <qtUtils.h>

#include "Notifications.h"

namespace MoonGlare {
namespace Editor {

SettingsWindow::SettingsWindow(QWidget *parent)
	: QDialog(parent)
{
	SetSettingID("SettingsWindow");
	m_Ui = std::make_unique<Ui::SettingsWindow>();
	m_Ui->setupUi(this);
	
	//m_Model = std::make_unique<QtShared::DataModels::StructureEditingModel>(this);
	//m_Model->SetTreeView(m_Ui->treeView);

	m_Ui->treeView->SetSettingID("treeView");

	auto &conf = GetSettings().getConfiguration();
	auto info = TypeEditor::Structure::GetStructureInfo(conf.GetTypeName());
	if (info) {
		auto SettingsStructure = info->m_CreateFunc(nullptr, nullptr);
		SettingsStructure->SetExternlDataSink(&conf);
		m_Ui->treeView->SetStructure(std::move(SettingsStructure));
	} else {
		AddLogf(Error, "Failed to get settings structure description!");
	}

	connect(m_Ui->pushButtonClose, &QPushButton::clicked, [this]() {
		close();
	});

	LoadSettings();
	m_Ui->treeView->Refresh();
}

SettingsWindow::~SettingsWindow() {
	SaveSettings();
	Notifications::SendSettingsChanged();
	m_Ui.release();
}

bool SettingsWindow::DoSaveSettings(pugi::xml_node node) const {
	SaveGeometry(node, this, "Qt:Geometry");
	SaveChildSettings(node, m_Ui->treeView);
	return true;
}

bool SettingsWindow::DoLoadSettings(const pugi::xml_node node) {
	LoadGeometry(node, this, "Qt:Geometry");
	LoadChildSettings(node, m_Ui->treeView);
	return true;
}

//----------------------------------------------------------------------------------

void SettingsWindow::showEvent(QShowEvent * event) {
	event->accept();
}

void SettingsWindow::closeEvent(QCloseEvent * event) {
	event->accept();
}

//----------------------------------------------------------------------------------

} //namespace Editor
} //namespace MoonGlare
