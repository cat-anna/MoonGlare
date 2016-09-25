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
	
	m_Model = std::make_unique<QStandardItemModel>();
	m_Model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_Model->setHorizontalHeaderItem(1, new QStandardItem("Values"));
	m_Model->setHorizontalHeaderItem(2, new QStandardItem("Description"));
	m_Ui->treeView->setModel(m_Model.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
//	m_Ui->treeView->setRootDecorated(false);
	m_Ui->treeView->setItemDelegate(new TypeEditor::CustomEditorItemDelegate(this));
	m_Ui->treeView->setColumnWidth(0, 200);
	m_Ui->treeView->setColumnWidth(1, 100);
	m_Ui->treeView->setColumnWidth(2, 100);

	auto &conf = GetSettings().getConfiguration();
	auto info = TypeEditor::Structure::GetStructureInfo(conf.GetTypeName());
	if (info) {
		m_SettingsStructure = info->m_CreateFunc(nullptr, nullptr);
		m_SettingsStructure->SetExternlDataSink(&conf);
	} else {
		AddLogf(Error, "Failed to get settings structure descruption!");
	}

	connect(m_Ui->pushButtonClose, &QPushButton::clicked, [this]() {
		close();
	});

	LoadSettings();
	Refresh();
}

SettingsWindow::~SettingsWindow() {
	SaveSettings();
	Notifications::SendSettingsChanged();
	m_Ui.release();
}

bool SettingsWindow::DoSaveSettings(pugi::xml_node node) const {
	SaveColumns(node, "treeViewDetails:Columns", m_Ui->treeView, 3);
	SaveGeometry(node, this, "Qt:Geometry");
	//SaveState(node, this, "Qt:State");
	return true;
}

bool SettingsWindow::DoLoadSettings(const pugi::xml_node node) {
	LoadColumns(node, "treeViewDetails:Columns", m_Ui->treeView, 3);
	LoadGeometry(node, this, "Qt:Geometry");
	//LoadState(node, this, "Qt:State");
	return true;
}

//----------------------------------------------------------------------------------

void SettingsWindow::Refresh() {
	if (!m_SettingsStructure)
		return;

	m_Model->removeRows(0, m_Model->rowCount());
//	m_CurrentComponent = EditableComponentValueInfo();
//	if (!m_CurrentItem)
//		return;

	QStandardItem *root = m_Model->invisibleRootItem();

	for (auto &value : m_SettingsStructure->GetValues()) {
		QStandardItem *CaptionElem = new QStandardItem(value->GetName().c_str());
		CaptionElem->setFlags(CaptionElem->flags() & ~Qt::ItemIsEditable);

//		EditableComponentValueInfo ecvi;
//		ecvi.m_OwnerComponent = component.get();
//		ecvi.m_Item = CaptionElem;
//		ecvi.m_ValueInterface = value.get();
//		ecvi.m_EditableEntity = m_CurrentItem.m_EditableEntity;
//
		QStandardItem *ValueElem = new QStandardItem();
		ValueElem->setData(value->GetValue().c_str(), Qt::EditRole);
		auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(value->GetTypeName());
		if (einfoit) {
			ValueElem->setData(einfoit->ToDisplayText(value->GetValue()).c_str(), Qt::DisplayRole);
		} else {
			ValueElem->setData(value->GetValue().c_str(), Qt::DisplayRole);
		}

//		CaptionElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
		ValueElem->setData(QVariant::fromValue(value.get()), TypeEditor::CustomEditorItemDelegate::QtRoles::StructureValue);

		QList<QStandardItem*> cols;
		cols << CaptionElem;
		cols << ValueElem;
		root->appendRow(cols);
	}

	m_Ui->treeView->collapseAll();
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
