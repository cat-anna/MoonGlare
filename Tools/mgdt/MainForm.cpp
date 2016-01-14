#include PCH_HEADER
#include "mgdtSettings.h"
#include "MainForm.h"
#include "ui_MainForm.h"

#include "LuaEditor/LuaWindow.h"
#include "ResourceEditor/ResourceBrowser.h"
#include "QuickActions/QuickActions.h"
#include "LogWindow/LogWindow.h"

class EngineInfoRequest : public RemoteConsoleObserver {
public:
	EngineInfoRequest(RemoteConsoleRequestQueue *Queue): RemoteConsoleObserver(InsiderApi::MessageTypes::InfoRequest, Queue) {
		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineVersion, "");
		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineExeName, "");
		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineBuildDate, "");
	}

	virtual void BuildMessage(InsiderApi::InsiderMessageBuffer &buffer) override { };

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override {
		if(m_Queue)
			m_Queue->RequestFinished(this);
		//auto *hdr = 
			message.GetAndPull<InsiderApi::PayLoad_InfoResponse>();

		auto ver = message.PullString();
		auto exen = message.PullString();
		auto buildd = message.PullString();

		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineVersion, ver);
		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineExeName, exen);
		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineBuildDate, buildd);

		return HanderStatus::Remove;
	};
private:
};

class EnginePingPong : public RemoteConsoleObserver {
public:
	enum class EngineState {
		Unknown, Active, Broken,
	};

	EnginePingPong(RemoteConsoleRequestQueue *Queue): RemoteConsoleObserver(InsiderApi::MessageTypes::Ping, Queue) {
	}

	virtual void BuildMessage(InsiderApi::InsiderMessageBuffer &buffer) override {
	};

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override {
		m_Queue->RequestFinished(this);
		ChangeState(EngineState::Active);

		auto sthis = shared_from_this();
		QTimer::singleShot(m_Timeout, [sthis, this]() {
			m_Queue->QueueRequest(sthis);
		});

		return HanderStatus::Remove;
	};

	virtual void OnSend() override  {
		RemoteConsoleObserver::OnSend();
		m_SendTime = std::chrono::steady_clock::now();
	}
	virtual void OnRecive() override {
		RemoteConsoleObserver::OnRecive();
		auto t = std::chrono::steady_clock::now();
		std::chrono::duration<double> sec = t - m_SendTime;
		char buf[64];
		sprintf_s(buf, "%d ms", static_cast<int>(sec.count() * 1000));
		MainForm::Get()->EngineStateValueChanged(EngineStateValue::EnginePing, buf);
	}

	virtual TimeOutAction TimedOut() override {
		ChangeState(EngineState::Broken);
		return TimeOutAction::Resend;
	}

	void ChangeState(EngineState val) {
		if (m_EngineState == val)
			return;
		m_EngineState = val;

		if (m_EngineState == EngineState::Active) {
			MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineState, "Active", ":/mgdt/icons/blue_ok.png");
			m_Queue->QueueRequest(std::make_shared<EngineInfoRequest>(m_Queue));
		} else {
			MainForm::Get()->EngineStateValueChanged(EngineStateValue::EngineState, "Broken", ":/mgdt/icons/red_flag.png");
			m_Queue->QueueRequest(std::make_shared<EngineInfoRequest>(nullptr));
			MainForm::Get()->EngineStateValueChanged(EngineStateValue::EnginePing, "");
		}
	}
private:
	std::chrono::steady_clock::time_point m_SendTime;
	EngineState m_EngineState = EngineState::Unknown;
};

//-----------------------------------------
//-----------------------------------------

static MainForm *_Instance = nullptr;

MainForm::MainForm(QWidget *parent)
	: QMainWindow(parent)
{
	_Instance = this;
	ui = new Ui::MainForm();
	ui->setupUi(this);

	connect(RemoteConsole::get(), SIGNAL(StateChanged(RemoteConsoleState)), SLOT(RemoteConsoleStateChanged(RemoteConsoleState)));

	connect(ui->actionScript_editor, SIGNAL(triggered()), SLOT(ShowScriptEditor()));
	connect(ui->actionBrowse_resources, SIGNAL(triggered()), SLOT(ShowResourceBrowser()));
	connect(ui->actionShow_quick_actions, SIGNAL(triggered()), SLOT(ShowQuickActions()));
	connect(ui->actionShow_logs, SIGNAL(triggered()), SLOT(ShowLogWindow()));

	auto &settings = mgdtSettings::get();
	settings.Window.MainForm.Apply(this);

	m_StateModelView = std::make_unique<QStandardItemModel>();
	m_StateModelView->setHorizontalHeaderItem(0, new QStandardItem("Parameter"));
	m_StateModelView->setHorizontalHeaderItem(1, new QStandardItem("Value"));

	ui->treeViewState->setModel(m_StateModelView.get());
	ui->treeViewState->setSelectionMode(QAbstractItemView::SingleSelection);

	ui->treeViewState->setColumnWidth(0, 150);
	ui->treeViewState->setColumnWidth(1, 100);

	QueueRequest(std::make_shared<EnginePingPong>(this));
}

MainForm::~MainForm() {
	auto &settings = mgdtSettings::get();
	settings.Window.MainForm.Store(this);
	delete ui;
}

MainForm* MainForm::Get() {
	return _Instance;
}

//-----------------------------------------

void MainForm::showEvent(QShowEvent * event) {
	auto &settings = mgdtSettings::get();

	static bool WindowsRestored = false;
	if (!WindowsRestored) {
		WindowsRestored = true;
		if (settings.Window.LuaEditor.Opened)
			ShowScriptEditor();
		if (settings.Window.ResourceBrowser.Opened)
			ShowResourceBrowser();
		if (settings.Window.QuickActions.Opened)
			ShowQuickActions();
		ShowResourceBrowser();
		if (settings.Window.LogWindow.Opened)
			ShowLogWindow();
	}
}

void MainForm::closeEvent(QCloseEvent * event) {
	event->ignore();

	bool LuaEditor = m_LuaEditor.get() != nullptr;
	bool ResEditor = m_ResourceBrowser.get() != nullptr;
	bool qactions = m_QuickActions.get() != nullptr;
	bool logwindow = m_LogWindow.get() != nullptr;

	if (m_LuaEditor) 
		if (!m_LuaEditor->close())
			return;
	
	if (m_ResourceBrowser) 
		if (!m_ResourceBrowser->close())
			return;

	if (m_QuickActions)
		if (!m_QuickActions->close())
			return;

	if (m_LogWindow)
		if (!m_LogWindow->close())
			return;

	auto &settings = mgdtSettings::get();
	settings.Window.LuaEditor.Opened = LuaEditor;
	settings.Window.ResourceBrowser.Opened = ResEditor;
	settings.Window.QuickActions.Opened = qactions;
	settings.Window.LogWindow.Opened = logwindow;

	event->accept();
}

//-----------------------------------------
	
void MainForm::ShowResourceBrowser() {
	if (!m_ResourceBrowser) {
		m_ResourceBrowser = std::make_unique<ResourceBrowser>(nullptr);
		connect(m_ResourceBrowser.get(), SIGNAL(WindowClosed()), SLOT(ResourceBrowserClosed()));
	}
	m_ResourceBrowser->show();
	m_ResourceBrowser->activateWindow();
}

void MainForm::ResourceBrowserClosed() {
	m_ResourceBrowser.reset();
}

void MainForm::ShowScriptEditor() {
	if (!m_LuaEditor) {
		m_LuaEditor = std::make_unique<LuaWindow>(nullptr);
		connect(m_LuaEditor.get(), SIGNAL(WindowClosed()), SLOT(ScriptEditorClosed()));
	}
	m_LuaEditor->show();
	m_LuaEditor->activateWindow();
}

void MainForm::ScriptEditorClosed() {
	m_LuaEditor.reset();
}

void MainForm::ShowQuickActions() { 
	if (!m_QuickActions) {
		m_QuickActions = std::make_unique<QuickActions>(nullptr);
		connect(m_QuickActions.get(), SIGNAL(WindowClosed()), SLOT(ScriptEditorClosed()));
	}
	m_QuickActions->show();
	m_QuickActions->activateWindow();
}

void MainForm::QuickActionsClosed()	{ 
	m_QuickActions.reset();
}

void MainForm::ShowLogWindow() {
	if (!m_LogWindow) {
		m_LogWindow = std::make_unique<LogWindow>(nullptr);
		connect(m_LogWindow.get(), SIGNAL(WindowClosed()), SLOT(LogWindowClosed()));
	}
	m_LogWindow->show();
	m_LogWindow->activateWindow();
}

void MainForm::LogWindowClosed() {
	m_LogWindow.reset();
}

void MainForm::EngineStateValueChanged(EngineStateValue type, const QString &Value, const QString& icon) {
	auto *item = m_StateValues[type];
	
	auto root = m_StateModelView->invisibleRootItem();
	if (!item) {
		if (Value.isEmpty())
			return;

		//auto groupname = EngineStateValueGroupsValues::ToString(type);
		QStandardItem *group;
		//if (groupname.empty() || !(group = m_StateGroupValues[groupname]))
			group = root;
		//else {
			//QList<QStandardItem*> cols;
			//cols << (group = new QStandardItem(groupname.c_str()));
			//root->appendRow(cols);
		//}

		QList<QStandardItem*> cols;
		cols << (item = new QStandardItem(EngineStateValueCaptionValues::ToString(type).c_str()));
		cols << new QStandardItem(Value);
		group->appendRow(cols);
		item->setData(QIcon(icon), Qt::DecorationRole);

		group->sortChildren(0);
		m_StateValues[type] = item;
		return;
	}

	int row = item->row();
	if (Value.isEmpty()) {
		m_StateValues[type] = nullptr;
		auto parent = item->parent();
		if(parent)
			parent->removeRow(row);
		else
			m_StateModelView->invisibleRootItem()->removeRow(row);
		return;
	}

	root->child(row, 0)->setData(QIcon(icon), Qt::DecorationRole);
	root->child(row, 1)->setData(Value, Qt::DisplayRole);
}

void MainForm::RemoteConsoleStateChanged(RemoteConsoleState state) {
	switch (state) {
		case RemoteConsoleState::Unknown:
			ui->statusBar->showMessage("Unknown");
			return;
		case RemoteConsoleState::NotStarted:
			ui->statusBar->showMessage("NotStarted");
			return;
		case RemoteConsoleState::Starting:
			ui->statusBar->showMessage("Starting");
			return;
		case RemoteConsoleState::Ready:
			ui->statusBar->showMessage("Ready");
			return;
		case RemoteConsoleState::Working:
			ui->statusBar->showMessage("Working");
			return;
		case RemoteConsoleState::Broken:
			ui->statusBar->showMessage("Broken");
			return;
		case RemoteConsoleState::Stopping:
			ui->statusBar->showMessage("Stopping");
			return;
	default:
		break;
	}
}

//-----------------------------------------

void MainForm::OpenEngineScript(const QString &file) {
	ShowScriptEditor();
	m_LuaEditor->OpenEngineScirpt(file);
	m_LuaEditor->activateWindow();
}
