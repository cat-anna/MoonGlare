#include PCH_HEADER
#include "mgdtSettings.h"
#include "ui_QuickActions.h"
#include "QuickActions.h"
#include "QuickActionEditor.h"
#include <DockWindowInfo.h>

struct QuickActionsInfo : public DockWindowInfo {
	virtual std::shared_ptr<DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<QuickActions>(parent);
	}

	QuickActionsInfo() {
		SetSettingID("QuickActionsInfo");
		SetDisplayName(tr("Quick actions"));
		SetShortcut("F5");
	}
};
DockWindowClassRgister::Register<QuickActionsInfo> QuickActionsInfoReg("QuickActions");

QuickActions::QuickActions(QWidget *parent)
	: DockWindow(parent)
{
	SetSettingID("QuickActions");
	ui = new Ui::QuickActions();
	ui->setupUi(this);

	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeView->setExpandsOnDoubleClick(false);

	ui->treeView->addAction(ui->actionExecute_quick_action);
	auto a = new QAction(this); a->setSeparator(true);
	ui->treeView->addAction(a);
	ui->treeView->addAction(ui->actionModify_quick_action);
	ui->treeView->addAction(ui->actionRemove_quick_action);

	connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ActionDoubleClicked(const QModelIndex&)));
	connect(ui->actionNew_quick_action, SIGNAL(triggered()), SLOT(NewAction()));
	connect(ui->actionRemove_quick_action, SIGNAL(triggered()), SLOT(RemoveAction()));
	connect(ui->actionModify_quick_action, SIGNAL(triggered()), SLOT(ModifyAction()));
	connect(ui->actionExecute_quick_action, SIGNAL(triggered()), SLOT(ExecuteAction()));

	RefreshView();
}

QuickActions::~QuickActions() {
	auto &settings = mgdtSettings::get();
	delete ui;
}

bool QuickActions::DoSaveSettings(pugi::xml_node node) const {
	DockWindow::DoSaveSettings(node);
	return true;
}

bool QuickActions::DoLoadSettings(const pugi::xml_node node) {
	DockWindow::DoLoadSettings(node);
	return true;
}

//-----------------------------------------

void QuickActions::RefreshView() {
	auto &settings = mgdtSettings::get();
	ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Script"));

	ui->treeView->setModel(m_ViewModel.get());
	ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);

	ui->treeView->setColumnWidth(0, 150);
	ui->treeView->setColumnWidth(1, 150);

	std::unordered_map<std::string, QStandardItem*> groups;

	auto root = m_ViewModel->invisibleRootItem();
	for (auto &it : settings.QuickActions.QuickActionList) {
		QList<QStandardItem*> cols;
		cols << new QStandardItem(it.Name.c_str());
		cols << new QStandardItem(it.Script.c_str());

		QStandardItem *group;
		if (it.Group.empty())
			group = root;
		else
			group = groups[it.Group];
		if (!group) {
			QList<QStandardItem*> groupcols;
			groupcols << (group = new QStandardItem(it.Group.c_str()));
			root->appendRow(groupcols);
			groups[it.Group] = group;
		}

		group->appendRow(cols);
	}

	for (auto &it : groups)
		it.second->sortChildren(0);
	root->sortChildren(0);
}

void QuickActions::NewAction() {
	QuickActionEditor editor(this);
	if (editor.exec() == QDialog::Rejected)
		return;

	QuickActionEditor::Action_t act;
	editor.Get(act);

	auto &settings = mgdtSettings::get();
	settings.QuickActions.QuickActionList.push_back(act);
	RefreshView();
}

void QuickActions::RemoveAction() {
	auto index = ui->treeView->selectionModel()->currentIndex();
	int r = index.row();
	auto parent = index.parent();
	auto item = parent.isValid() ? parent.child(r, 0) : index.sibling(r, 0);
	if (!item.isValid())
		return;
	std::string str = item.data(Qt::DisplayRole).toString().toUtf8().constData();
	auto &settings = mgdtSettings::get();
	settings.QuickActions.QuickActionList.remove_if([&str](const mgdtSettings::QuickActions_t::Action_t &a)->bool {
		return str == a.Name;
	});
	RefreshView();
}

void QuickActions::ModifyAction() {
	auto index = ui->treeView->selectionModel()->currentIndex();
	int r = index.row();
	
	auto parent = index.parent();
	auto item = parent.isValid() ? parent.child(r, 0) : index.sibling(r, 0);
	if (!item.isValid())
		return;

	std::string str = item.data(Qt::DisplayRole).toString().toUtf8().constData();
	auto &settings = mgdtSettings::get().QuickActions;

	auto *action = settings.FindByName(str);
	if (!action)
		return;

	QuickActionEditor editor(this);
	editor.Set(*action);
	if (editor.exec() == QDialog::Rejected)
		return;
	editor.Get(*action);
	RefreshView();
}

void QuickActions::ExecuteAction() {
	auto index = ui->treeView->selectionModel()->currentIndex();
	int r = index.row();

	auto parent = index.parent();
	auto item = parent.isValid() ? parent.child(r, 1) : index.sibling(r, 1);
	if(!item.isValid())
		return;

	auto str = item.data(Qt::DisplayRole).toString();

	//ui.tabWidget->setCurrentIndex(ui.tabWidget->indexOf(tab));
	//QString str = idx.data(Qt::DisplayRole).toString();
	//MainForm::Get()->OpenEngineScript(str);
	GetRemoteConsole().ExecuteCode(str);
}

void QuickActions::ActionDoubleClicked(const QModelIndex& index) {
	ExecuteAction();
}
