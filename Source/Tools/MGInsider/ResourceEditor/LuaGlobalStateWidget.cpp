#include PCH_HEADER
#include "LuaGlobalStateWidget.h"
#include "ui_LuaGlobalStateWidget.h"
#include "mgdtSettings.h"
#include <RemoteConsole.h>

class LuaGlobalStateWidget::LuaRequest : public RemoteConsoleEnumerationObserver {
public:
	LuaRequest(const char *location, QStandardItem *parent, LuaGlobalStateWidget *Owner, bool Recursive = true):
			RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes::EnumerateLua, location),
			m_Parent(parent),
			m_Recursive(Recursive),
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
			auto *item = message.GetAndPull<InsiderApi::PayLoad_LuaElement_Item>();

			const char *name = message.PullString();
			const char *value = message.PullString();

			//QStandardItem * row = new QStandardItem();
			QList<QStandardItem*> cols;
			QStandardItem *first;
			cols << (first = new QStandardItem(name));
			cols << new QStandardItem(ToStringType(item->LuaType));
			cols << new QStandardItem(value);
			m_Parent->appendRow(cols);

			if (!strcmp("_G", name))
				continue;

			if (m_Recursive && item->LuaType == 5)  {//table
				std::string loc = GetLocation();
				if (std::string(name) == std::to_string(strtoul(name, nullptr, 10)))
					loc += std::string("[") + name + "]";
				else	  
					loc += std::string("['") + name + "']";
				m_Owner->QueueRequest(SharedRequest(new LuaRequest(loc.c_str(), first, m_Owner)));

				QList<QStandardItem*> cols;
				QStandardItem *firstmeta;
				cols << (firstmeta = new QStandardItem("__metatable"));
				first->appendRow(cols);
				loc = "getmetatable(" + loc + ")";
				m_Owner->QueueRequest(SharedRequest(new LuaRequest(loc.c_str(), firstmeta, m_Owner, false)));
			}
		}
		m_Parent->sortChildren(0);
		m_Owner->RequestFinished(this);
		return HanderStatus::Remove; 
	};
private:
	QStandardItem *m_Parent;
	LuaGlobalStateWidget *m_Owner;
	bool m_Recursive;
};

//-----------------------------------------
//-----------------------------------------

static ResourceEditorTabRegister::Register<LuaGlobalStateWidget> _Register("LuaGlobalStateWidget");

LuaGlobalStateWidget::LuaGlobalStateWidget(QWidget *parent)
	: ResourceEditorBaseTab(parent) {
	SetName("Lua global state");
	ui = new Ui::LuaGlobalStateWidget();
	ui->setupUi(this);
	
	//connect(ui.ButtonRefresh, SIGNAL(clicked(bool)), SLOT(Refresh()));
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	SetQueueName("Lua state list");

	ResetTreeView();
}

LuaGlobalStateWidget::~LuaGlobalStateWidget() {
	ResetTreeView();
	delete ui;
}

//-----------------------------------------

void LuaGlobalStateWidget::ResetTreeView() {
	auto &settings = mgdtSettings::get();
	if (m_ViewModel) {
		//settings.LuaGlobalBrowser.TreeCollumnWidth = ui->treeView->columnWidth(0);
		//settings.LuaGlobalBrowser.TypeCollumnWidth = ui->treeView->columnWidth(1);
		//settings.LuaGlobalBrowser.ValueCollumnWidth = ui->treeView->columnWidth(2);
	}
    ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Tree"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Type"));
	m_ViewModel->setHorizontalHeaderItem(2, new QStandardItem("Value"));

    ui->treeView->setModel(m_ViewModel.get());
	  
	ui->treeView->setColumnWidth(0, 100);
	ui->treeView->setColumnWidth(1, 100);
	ui->treeView->setColumnWidth(2, 100);
}

//-----------------------------------------

void LuaGlobalStateWidget::Refresh() {
	CancelRequests();
	ResetTreeView();

	auto request = std::make_shared<LuaRequest>("_G", m_ViewModel->invisibleRootItem(), this);
	QueueRequest(request);
}
