#include PCH_HEADER

#include "StructureEditingModel.h"
#include <TypeEditor/CustomEditorItemDelegate.h>

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

StructureEditingModel::StructureEditingModel(QWidget *parent)
	: QTreeView(parent)
{
	m_Model = std::make_unique<QStandardItemModel>();
	m_Model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_Model->setHorizontalHeaderItem(1, new QStandardItem("Values"));
	m_Model->setHorizontalHeaderItem(2, new QStandardItem("Description"));
	connect(m_Model.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(ItemChanged(QStandardItem *)));

	setModel(m_Model.get());
	setSelectionMode(QAbstractItemView::SingleSelection);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setItemDelegate(new TypeEditor::CustomEditorItemDelegate(moduleManager, this));
	setColumnWidth(0, 200);
	setColumnWidth(1, 100);
	setColumnWidth(2, 100);
}

StructureEditingModel::~StructureEditingModel()
{
}

void StructureEditingModel::SetModuleManager(QtShared::SharedModuleManager mm) {
    moduleManager.swap(mm);
    setItemDelegate(new TypeEditor::CustomEditorItemDelegate(moduleManager, this));
}

//-----------------------------------------

void StructureEditingModel::CreateStructure(TypeEditor::SharedStructureInfo structure) {
	SetStructure(structure->m_CreateFunc(nullptr));
}

void StructureEditingModel::SetStructure(TypeEditor::UniqueStructure&& structure) {
	m_Structure.swap(structure);
	Refresh();
}

bool StructureEditingModel::DoSaveSettings(pugi::xml_node node) const {
	SaveColumns(node, "This:Columns", this, 3);
	return true;
}

bool StructureEditingModel::DoLoadSettings(const pugi::xml_node node) {
	LoadColumns(node, "This:Columns", this, 3);
	return true;
}

void StructureEditingModel::ItemChanged(QStandardItem *item) {
	SetModiffiedState(true);
}

void StructureEditingModel::Refresh() {
	m_Model->removeRows(0, m_Model->rowCount());

	if (!m_Structure)
		return;

	//	m_CurrentComponent = EditableComponentValueInfo();
	//	if (!m_CurrentItem)
	//		return;

	QStandardItem *root = m_Model->invisibleRootItem();

	for (auto &value : m_Structure->GetValues()) {
		QStandardItem *CaptionElem = new QStandardItem(value->GetName().c_str());
		CaptionElem->setFlags(CaptionElem->flags() & ~Qt::ItemIsEditable);

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

	collapseAll();
}

//-----------------------------------------

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare
