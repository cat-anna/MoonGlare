#include PCH_HEADER
#include "ScriptListWidget.h"
#include "ui_ScriptListWidget.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

class ScriptListWidget::LuaRequest : public RemoteConsoleEnumerationObserver {
public:
	LuaRequest(QStandardItem *parent, ScriptListWidget *Owner):
			RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes::EnumerateScripts, ""),
			m_Parent(parent),
			m_Owner(Owner) {
	}

	static const char* ToStringType(int v) {
		switch (v) {
		case 0: return "nil";
		case 1: return "boolean";
		case 2: return "light user data";
		case 3: return "number";
		case 4: return "string";
		case 5: return "table";
		case 6: return "function";
		case 7: return "user data";
		case 8: return "thread";
		default:
			return "?";
		}
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override { 
		auto hdr = message.GetAndPull<InsiderApi::PayLoad_ListBase>();

		for (unsigned i = 0; i < hdr->Count; ++i) {
			auto *item = message.GetAndPull<InsiderApi::PayLoad_ScriptList_Item>();
			
			const char *name = message.PullString();
			const char *type;
			switch (item->Type) {
			case InsiderApi::PayLoad_ScriptList_Item::ItemType::BroadcastedCode:
				type = "Broadcasted";
				break;
			case InsiderApi::PayLoad_ScriptList_Item::ItemType::ScriptFile:
				type = "File";
				break;
			default:
				type = "Unknown";
				break;
			}
			char size[64];
			sprintf_s(size, "%.2f kb", item->DataLen / 1024.0f);

			QList<QStandardItem*> cols;
			cols << new QStandardItem(name);
			cols << new QStandardItem(type);
			cols << new QStandardItem(size);
			m_Parent->appendRow(cols);
		}
		m_Owner->RequestFinished(this);
		m_Parent->sortChildren(0);
		return HanderStatus::Remove; 
	};
private:
	QStandardItem *m_Parent;
	ScriptListWidget *m_Owner;
};

//-----------------------------------------
//-----------------------------------------

static ResourceEditorTabRegister::Register<ScriptListWidget> _Register("ScriptListWidget");

ScriptListWidget::ScriptListWidget(QWidget *parent)
	: ResourceEditorBaseTab(parent) {
	SetName("Scripts");
	ui = new Ui::ScriptListWidget();
	ui->setupUi(this);
	
	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeView->setExpandsOnDoubleClick(false);

	m_EditScriptAction = new QAction(QIcon(":/mgdt/icons/blue_down_arrow.png"), "Edit script", this);
	connect(m_EditScriptAction, SIGNAL(triggered()), SLOT(EditScriptAction()));
	connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ScriptDoubleClicked(const QModelIndex&)));

	ui->treeView->addAction(m_EditScriptAction);

	ResetTreeView();
}

ScriptListWidget::~ScriptListWidget() {
	ResetTreeView();
	delete ui;
}

//-----------------------------------------

void ScriptListWidget::ResetTreeView() {
//	auto &settings = mgdtSettings::get();
    ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Type"));
	m_ViewModel->setHorizontalHeaderItem(2, new QStandardItem("Size"));

    ui->treeView->setModel(m_ViewModel.get());
	ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	  
	ui->treeView->setColumnWidth(0, 400);
	ui->treeView->setColumnWidth(1, 50);
	ui->treeView->setColumnWidth(2, 50);
}

//-----------------------------------------

void ScriptListWidget::Refresh() {
	CancelRequests();
	ResetTreeView();

	auto request = std::make_shared<LuaRequest>(m_ViewModel->invisibleRootItem(), this);
	QueueRequest(request);
}

//-----------------------------------------

void ScriptListWidget::EditScriptAction() {
	//ui->treeView->se
	auto idx = ui->treeView->selectionModel()->currentIndex();
	QString str = idx.data(Qt::DisplayRole).toString();
	MainForm::Get()->OpenEngineScript(str);
}

//-----------------------------------------

void ScriptListWidget::ScriptDoubleClicked(const QModelIndex& index) {
	EditScriptAction();
}
