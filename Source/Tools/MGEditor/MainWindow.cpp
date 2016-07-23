#include PCH_HEADER
#include <qobject.h>
#include "EditorSettings.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <DockWindow.h>
#include <qtUtils.h>

#include "Module/DataModule.h"
#include "Notifications.h"
#include "FileSystem.h"

namespace MoonGlare {
namespace Editor {

static MainWindow *_Instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	_Instance = this;
	SetSettingID("MainWindow");
	m_Settings.ResetToDefault();
	m_Ui = std::make_unique<Ui::MainWindow>();
	m_Ui->setupUi(this);

	m_FileSystem = std::make_shared<FileSystem>();
	
	connect(m_Ui->actionNew, SIGNAL(triggered()), SLOT(NewModuleAction()));
	connect(m_Ui->actionOpen, SIGNAL(triggered()), SLOT(OpenModuleAction()));
	connect(m_Ui->actionClose, SIGNAL(triggered()), SLOT(CloseModuleAction()));
	connect(m_Ui->actionExit, SIGNAL(triggered()), SLOT(CloseEditorAction()));

	m_DockWindows.reserve(256);//because why not
	QtShared::DockWindowClassRgister::GetRegister()->Enumerate([this](auto &ci) {
		auto ptr = ci.SharedCreate(this);
		m_DockWindows.push_back(ptr);

		m_Ui->menuWindows->addAction(ptr->GetIcon(), ptr->GetDisplayName(), ptr.get(), SLOT(Show()), ptr->GetKeySequence());
		AddLogf(Info, "Registered DockWindow: %s", ci.Alias.c_str());

		ptr->LoadSettings();
	});
	m_DockWindows.shrink_to_fit();

	LoadSettings();
}

MainWindow::~MainWindow() {
	SaveSettings();
	for (auto &it : m_DockWindows)
		it->SaveSettings();
	m_DockWindows.clear();
	m_Ui.release();
}

MainWindow* MainWindow::Get() {
	return _Instance;
}

bool MainWindow::DoSaveSettings(pugi::xml_node node) const {
	SaveGeometry(node, this, "Qt:Geometry");
	SaveState(node, this, "Qt:State");
	m_Settings.Write(node, "Settings");
	return true;
}

bool MainWindow::DoLoadSettings(const pugi::xml_node node) {
	LoadGeometry(node, this, "Qt:Geometry");
	LoadState(node, this, "Qt:State");
	m_Settings.Read(node, "Settings");

	QTimer::singleShot(500, [this] {
		if (m_Settings.m_LoadLastModule && !m_Settings.m_LastModule.empty())
			OpenModule(m_Settings.m_LastModule);
	});

	return true;
}

void MainWindow::NewModuleAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
	}

	auto f = QFileDialog::getSaveFileName(this, "MoonGlare", QString(),
										  QtShared::Utils::GetMoonGlareProjectFilter());

	if (f.isEmpty())
		return;

	if (m_DataModule)
		CloseModule();
	NewModule(f.toLatin1().constData());
}

void MainWindow::OpenModuleAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
	}

	auto f = QFileDialog::getOpenFileName(this, "MoonGlare", QString(),
										  QtShared::Utils::GetMoonGlareProjectFilter());
	if (f.isEmpty())
		return;

	if (m_DataModule) 
		CloseModule();

	OpenModule(f.toLatin1().constData());
}

void MainWindow::CloseModuleAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
		CloseModule();
	}
}

void MainWindow::CloseEditorAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
		CloseModule();
	}

	close();
}

//-----------------------------------------

void MainWindow::NewModule(const std::string& MasterFile) {
	if (m_DataModule)
		return;

	m_DataModule = Module::DataModule::NewModule(MasterFile);
	if (!m_DataModule)
		return;
	m_Settings.m_LastModule = MasterFile;
	Notifications::SendProjectChanged(m_DataModule);
	AddLogf(Info, "New module: %s", MasterFile.c_str());
}

void MainWindow::OpenModule(const std::string& MasterFile)  {
	if (m_DataModule)
		return;

	QFileInfo check_file(MasterFile.c_str());
	if (!check_file.exists())
		return;

	m_DataModule = Module::DataModule::OpenModule(MasterFile);
	if (!m_DataModule)
		return;
	m_Settings.m_LastModule = MasterFile;
	Notifications::SendProjectChanged(m_DataModule);
	AddLogf(Info, "Open module: %s", MasterFile.c_str());
}

void MainWindow::CloseModule() {
	if (!m_DataModule)
		return;

	m_DataModule->SaveNow();
	m_DataModule.reset();

	Notifications::SendProjectChanged(m_DataModule);
}

//-----------------------------------------

void MainWindow::showEvent(QShowEvent * event) {
}

void MainWindow::closeEvent(QCloseEvent * event) {
	m_DataModule.reset();
	event->accept();
}

} //namespace Editor
} //namespace MoonGlare
