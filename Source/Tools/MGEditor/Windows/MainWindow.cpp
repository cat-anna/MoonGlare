#include PCH_HEADER
#include "MainWindow.h"
#include "EditorSettings.h"
#include "ui_MainWindow.h"
#include <qlabel.h>
#include <qobject.h>

#include <DockWindow.h>
#include <ToolBase/UserQuestions.h>

#include "DataModule.h"
#include "FileSystem.h"
#include "Notifications.h"

#include "ForegroundProcess.h"
#include "SettingsWindow.h"

#include "Build/BuildOptions.h"
#include "Build/BuildProcess.h"
#include "InputConfigurator.h"

namespace MoonGlare {
namespace Editor {

MoonGlare::ModuleClassRegister::Register<MainWindow> MainWindowReg("MainWindow");

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
        SettingsWindow sw(GetModuleManager(), this);
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

    connect(m_Ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::TabCloseRequested);
#ifdef DEBUG
    setWindowTitle(windowTitle() + " [DEBUG]"); 
#endif
}

MainWindow::~MainWindow() {
    m_Ui.reset();
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

    show();
    return true;
}

bool MainWindow::Finalize() {
    jobProcessor.reset();

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

    auto stt = GetModuleManager()->QuerryModule<EditorSettings>();

    auto conf = stt->GetConfiguration();
    auto state = stt->GetState();
    if (conf.m_LoadLastModule && !state.m_LastModule.empty()) {
        QTimer::singleShot(500, [this, stt] {
            auto state = stt->GetState();
            OpenModule(state.m_LastModule);
        });
    }
    return true;
}

void MainWindow::NewModuleAction() {
    if (m_DataModule) {
        if (!AskForPermission()) {
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
        if (!AskForPermission()) {
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
        if (!AskForPermission()) {
            return;
        }
        CloseModule();
    }
}

void MainWindow::CloseEditorAction() {
    if (m_DataModule) {
        if (!AskForPermission()) {
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

    auto pm = GetModuleManager()->QuerryModule<MoonGlare::Module::BackgroundProcessManager>();
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

    auto stt = GetModuleManager()->QuerryModule<EditorSettings>();

    stt->GetState().m_LastModule = MasterFile;
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

    auto stt = GetModuleManager()->QuerryModule<EditorSettings>();

    stt->GetState().m_LastModule = MasterFile;
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
    event->accept();
}

void MainWindow::closeEvent(QCloseEvent * event) {
    m_DataModule.reset();
    event->accept();       
}

//-----------------------------------------

void MainWindow::AddAction(std::string id, ActionVariant action, std::weak_ptr<iActionProvider> provider) {
    auto it = actionBarItems.find(id);
    if (it != actionBarItems.end()) {
        __debugbreak();
        return;
    }

    ActionInfo ai;
    ai.action = action;
    ai.provider = provider;

    QAction* ac = ai.GetActionPtr();

    if (ac == nullptr) {
        ai.action = m_Ui->mainToolBar->addSeparator();
    } else {
        m_Ui->mainToolBar->addAction(ac);
    }
    actionBarItems[id] = ai;
}

void MainWindow::RemoveProvider(std::weak_ptr<iActionProvider> provider) {
    for (auto it = actionBarItems.begin(), jt = actionBarItems.end(); it != jt;)    {
        const auto &item = it->second;
        if (item.provider.lock() == provider.lock()) {
            m_Ui->mainToolBar->removeAction(item.GetActionPtr());
            auto remove = it;
            ++it;
            actionBarItems.erase(remove);
        } else {
            ++it;
        }
    };
}

//-----------------------------------------

//iMainWindowTabsCtl
QWidget* MainWindow::GetTabParentWidget() const {
    return m_Ui->tabWidget;
}

void MainWindow::AddTab(const std::string &id, std::shared_ptr<iTabViewBase> tabWidget) {
    QWidget *w = dynamic_cast<QWidget*>(tabWidget.get());
    if (!w) {
        __debugbreak();
        return;
    }
    if (openedTabs.find(id) == openedTabs.end()) {
        openedTabs[id] = tabWidget;
        m_Ui->tabWidget->addTab(w, tabWidget->GetTabTitle().c_str());
        m_Ui->tabWidget->setCurrentWidget(w);
    }   
}

bool MainWindow::TabExists(const std::string &id) const {
    return openedTabs.find(id) != openedTabs.end();
}

void MainWindow::ActivateTabs(const std::string &id) {
    auto it = openedTabs.find(id);
    if (openedTabs.find(id) == openedTabs.end())
        return;
    QWidget *w = dynamic_cast<QWidget*>(it->second.get());
    if (!w) {
        __debugbreak();
        return;
    }
    m_Ui->tabWidget->setCurrentWidget(w);
}

void MainWindow::TabCloseRequested(int index) {
    auto w = m_Ui->tabWidget->widget(index);
    if (!w)
        return;

    for (auto &item : openedTabs) {
        QWidget *iw = dynamic_cast<QWidget*>(item.second.get());
        if (iw == w && item.second->CanClose()) {
            openedTabs.erase(item.first);
            m_Ui->tabWidget->removeTab(index);
            return;
        }
    }
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
