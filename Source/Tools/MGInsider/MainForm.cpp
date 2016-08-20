#include PCH_HEADER
#include <QSettings>
#include "mgdtSettings.h"
#include "MainForm.h"
#include "ui_MainForm.h"
#include "DockWindow.h"

#include <Notifications.h>

#include "LuaEditor/LuaWindow.h"
#include "ResourceEditor/ResourceBrowser.h"
#include "LogWindow/LogWindow.h"


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
	connect(ui->actionShow_logs, SIGNAL(triggered()), SLOT(ShowLogWindow()));

	auto &settings = mgdtSettings::get();
	settings.Window.MainForm.Apply(this);

	m_DockWindows.reserve(256);//because why not
	DockWindowClassRgister::GetRegister()->Enumerate([this](auto &ci) {
		auto ptr = ci.SharedCreate();
		m_DockWindows.push_back(ptr);

		ui->menuWindows->addAction(ptr->GetIcon(), ptr->GetDisplayName(), ptr.get(), SLOT(Show()), ptr->GetKeySequence());
		AddLogf(Info, "Registered DockWindow: %s", ci.Alias.c_str());

		ptr->LoadSettings();
	});


	{
		QSettings settings("S.ini", QSettings::IniFormat);
		restoreState(settings.value("MainWindow/State").toByteArray());
		restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
	}
}

MainForm::~MainForm() {
	{
		QSettings settings("S.ini", QSettings::IniFormat);
		settings.setValue("MainWindow/State", saveState());
		settings.setValue("MainWindow/Geometry", saveGeometry());
	}
	for (auto &it : m_DockWindows)
		it->SaveSettings();
	m_DockWindows.clear();
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
		ShowResourceBrowser();
		if (settings.Window.LogWindow.Opened)
			ShowLogWindow();
	}
}

void MainForm::closeEvent(QCloseEvent * event) {
	event->ignore();

	bool LuaEditor = m_LuaEditor.get() != nullptr;
	bool ResEditor = m_ResourceBrowser.get() != nullptr;
	bool logwindow = m_LogWindow.get() != nullptr;

	if (m_LuaEditor) 
		if (!m_LuaEditor->close())
			return;
	
	if (m_ResourceBrowser) 
		if (!m_ResourceBrowser->close())
			return;

	if (m_LogWindow)
		if (!m_LogWindow->close())
			return;

	auto &settings = mgdtSettings::get();
	settings.Window.LuaEditor.Opened = LuaEditor;
	settings.Window.ResourceBrowser.Opened = ResEditor;
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

//-----------------------------------------
