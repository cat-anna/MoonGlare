#include PCH_HEADER
#include <qobject.h>
#include <qlabel.h>
#include "EditorSettings.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <DockWindow.h>
#include <qtUtils.h>

#include "DataModule.h"
#include "Notifications.h"
#include "FileSystem.h"

#include "SettingsWindow.h"
#include "ForegroundProcess.h"

#include "Build/BuildOptions.h"
#include "Build/BuildProcess.h"
#include "InputConfigurator.h"

namespace MoonGlare {
namespace Editor {

static MainWindow *_Instance = nullptr;

MainWindow::MainWindow(SharedModuleManager modmgr)
    : QMainWindow(nullptr), iModule(std::move(modmgr))
{
    _Instance = this;
    SetSettingID("MainWindow");
    m_Ui = std::make_unique<Ui::MainWindow>();
    m_Ui->setupUi(this);

    connect(m_Ui->actionNew, SIGNAL(triggered()), SLOT(NewModuleAction()));
    connect(m_Ui->actionOpen, SIGNAL(triggered()), SLOT(OpenModuleAction()));
    connect(m_Ui->actionClose, SIGNAL(triggered()), SLOT(CloseModuleAction()));
    connect(m_Ui->actionExit, SIGNAL(triggered()), SLOT(CloseEditorAction()));
    connect(m_Ui->actionBuild, SIGNAL(triggered()), SLOT(BuildModuleAction()));
    connect(m_Ui->actionActionPackModule, SIGNAL(triggered()), SLOT(PackModuleAction()));
    connect(m_Ui->actionEditorConfiguration, &QAction::triggered, [this]() {
        SettingsWindow sw(this);
        sw.exec();
    } );

    connect(m_Ui->actionInputSettings, &QAction::triggered, [this]() {
        InputConfigurator ic(this, GetModuleManager(), m_DataModule->GetInputSettingsFile());
        ic.exec();
    });

    refreshTimer.setSingleShot(false);
    refreshTimer.setInterval(500);
    connect(&refreshTimer, &QTimer::timeout, this, &MainWindow::RefreshStatus);
    refreshTimer.start();

    jobProcessorStatus = new QLabel(nullptr);
    jobProcessorStatus->setText("Idle");
    jobProcessorStatus->setMinimumSize({100, 0});
    jobProcessorStatus->setAlignment(Qt::AlignCenter);
    m_Ui->statusBar->addPermanentWidget(jobProcessorStatus, 0);

    m_Ui->statusBar->showMessage("", 500);
}

MainWindow::~MainWindow() {
    m_Ui.release();
}

bool MainWindow::PostInit() {
    auto mm = GetModuleManager();
    jobProcessor = mm->QuerryModule<QtShared::iJobProcessor>();

    for (auto &item : mm->QuerryInterfaces<QtShared::BaseDockWindowModule>()) {
        if (item.m_Interface->IsMainMenu()) {
            m_Ui->menuWindows->addAction(item.m_Interface->GetIcon(), item.m_Interface->GetDisplayName(), item.m_Interface.get(), SLOT(Show()), item.m_Interface->GetKeySequence());
            AddLogf(Info, "Registered DockWindow: %s", item.m_Module->GetModuleName().c_str());
        }
    }

    return true;
}

void MainWindow::RefreshStatus() {   
    if (jobProcessor) {
        auto jobs = jobProcessor->GetQueuedJobCount();
        if (jobs > 0) {
            jobProcessorStatus->setText(("Queued Jobs: " + std::to_string(jobs)).c_str());
            jobProcessorStatus->setStyleSheet("QLabel { background-color : red; }");
        } else {
            jobProcessorStatus->setText("Idle");
            jobProcessorStatus->setStyleSheet("QLabel { }");
        }
    }
}

MainWindow* MainWindow::Get() {
    return _Instance;
}

bool MainWindow::DoSaveSettings(pugi::xml_node node) const {
    SaveGeometry(node, this, "Qt:Geometry");
    SaveState(node, this, "Qt:State");
    return true;
}

bool MainWindow::DoLoadSettings(const pugi::xml_node node) {
    LoadGeometry(node, this, "Qt:Geometry");
    LoadState(node, this, "Qt:State");

    auto conf = GetSettings().getConfiguration();
    auto state = GetSettings().getState();
    if (conf.m_LoadLastModule && !state.m_LastModule.empty()) {
        QTimer::singleShot(500, [this] {
            auto state = GetSettings().getState();
            OpenModule(state.m_LastModule);
        });
    }
    return true;
}

void MainWindow::NewModuleAction() {
    if (m_DataModule) {
        if (!QtShared::Utils::AskForPermission(this)) {
            return;
        }
    }

    auto f = QFileDialog::getExistingDirectory(this, "MoonGlare", QString());

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

    auto f = QFileDialog::getExistingDirectory(this, "MoonGlare", QString());
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

void MainWindow::BuildModuleAction() {
    BuildOptions w(this, m_DataModule, GetModuleManager());
    if (w.exec() != QDialog::Accepted)
        return;

    auto settings = w.GetSettings();
    settings.InputSettingsFile = m_DataModule->GetInputSettingsFile();
    settings.moduleSourceLocation = m_DataModule->GetBaseDirectory();
    settings.binLocation = std::string(QApplication::applicationDirPath().toLocal8Bit().data()) + "/";

    auto pm = GetModuleManager()->QuerryModule<QtShared::BackgroundProcessManager>();
    auto process = pm->CreateProcess<BuildProcess>(std::to_string(rand()), settings);
    ForegroundProcess fp(this, GetModuleManager(), process, false);
    fp.exec();
}

void MainWindow::PackModuleAction() {
    ReportNotImplemented();
    //BuildOptions w(this, GetModuleManager());
    //if (w.exec() != QDialog::Accepted)
        //return;

    //bs.outputLocation = ui->lineEditOutput->text().toLocal8Bit().begin();
    //bs.debugBuild = ui->checkBoxDebug->isChecked();
    //bs.disableValidation = ui->checkBoxDisableValidation->isChecked();

    //Editor::BuildOptions settings = w.GetSettings(
    //settings.
    //settings.moduleSourceLocation = m_DataModule->GetBaseDirectory();
    //settings.binLocation = std::string(QApplication::applicationDirPath().toLocal8Bit().data()) + "/";

    //auto pm = GetModuleManager()->QuerryModule<QtShared::BackgroundProcessManager>();
    //auto process = pm->CreateProcess<BuildProcess>(std::to_string(rand()), settings);
    //ForegroundProcess fp(this, GetModuleManager(), process, false);
    //fp.exec();
}

//-----------------------------------------

void MainWindow::NewModule(const std::string& MasterFile) {
    if (m_DataModule)
        return;

    m_DataModule = Module::DataModule::NewModule(MasterFile);
    if (!m_DataModule)
        return;

    GetSettings().getState().m_LastModule = MasterFile;
    Notifications::SendProjectChanged(m_DataModule);
    AddLogf(Info, "New module: %s", MasterFile.c_str());
}

void MainWindow::OpenModule(const std::string& MasterFile)  {
    if (m_DataModule)
        return;

    //QFileInfo check_file(MasterFile.c_str());
    //if (!check_file.exists())
        //return;

    m_DataModule = Module::DataModule::OpenModule(MasterFile);
    if (!m_DataModule)
        return;
    GetSettings().getState().m_LastModule = MasterFile;
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

//-----------------------------------------

    /*
void MainWindow::CreateFileEditor(const std::string & URI, std::shared_ptr<SharedData::FileCreatorInfo> info) {
    auto inst = info->m_DockEditor->GetInstance();
    auto editor = dynamic_cast<QtShared::iEditor*>(inst.get());
    if (!editor) {
        AddLogf(Error, "Fatal Error!");
        ErrorMessage("Fatal Error!");
        return;
    }

    if (editor->Create(URI, info->m_Info)) {
        inst->show();
    } else {
        ErrorMessage("Failed to create file!");
        AddLog(Error, "Failed to create file!");
    }
}
*/

} //namespace Editor
} //namespace MoonGlare
