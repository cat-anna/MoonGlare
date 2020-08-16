#include "main_window.hpp"
#include "modules/filesystem_module.hpp"
#include "ui_main_window.h"
#include <QFileDialog>
#include <algorithm>
#include <change_container.hpp>
#include <dock_window_info.hpp>
#include <filesystem>
#include <json_helpers.hpp>
#include <runtime_modules/application_settings.hpp>

// #include "EditorSettings.h"
// #include "FileSystem.h"
// #include "Notifications.h"
// #include "ForegroundProcess.h"
// #include "SettingsWindow.h"
// #include "Build/BuildOptions.h"
// #include "Build/BuildProcess.h"
// #include "InputConfigurator.h"

namespace MoonGlare::Tools::Editor::Dialogs {

void to_json(nlohmann::json &j, const MainWindowSettings &p) {
    j = {
        {"recent_modules", p.recent_modules},
        {"load_last_module", p.load_last_module},
    };
}

void from_json(const nlohmann::json &j, MainWindowSettings &p) {
    j.at("recent_modules").get_to(p.recent_modules);
    j.at("load_last_module").get_to(p.load_last_module);
}

//-----------------------------------------

MoonGlare::Tools::ModuleClassRegister::Register<MainWindow> MainWindowReg("MainWindow");

MainWindow::MainWindow(SharedModuleManager modmgr)
    : QMainWindow(nullptr), iModule(std::move(modmgr)) {
    SetSettingId("editor_main_window", 100);
    ui = std::make_unique<Ui::MainWindow>();
    ui->setupUi(this);

    connect(ui->actionNew, SIGNAL(triggered()), SLOT(NewModuleAction()));
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(OpenModuleAction()));
    connect(ui->actionClose, SIGNAL(triggered()), SLOT(CloseModuleAction()));
    connect(ui->actionExit, SIGNAL(triggered()), SLOT(CloseEditorAction()));

    //     connect(ui->actionBuild, SIGNAL(triggered()), SLOT(BuildModuleAction()));
    //     connect(ui->actionActionPackModule, SIGNAL(triggered()), SLOT(PackModuleAction()));
    //     connect(ui->actionEditorConfiguration, &QAction::triggered, [this]() {
    //         SettingsWindow sw(GetModuleManager(), this);
    //         sw.exec();
    //     });

    //     connect(ui->actionInputSettings, &QAction::triggered, [this]() {
    //         InputConfigurator ic(this, GetModuleManager(), m_DataModule->GetInputSettingsFile());
    //         ic.exec();
    //     });

    refresh_timer.setSingleShot(false);
    refresh_timer.setInterval(500);
    connect(&refresh_timer, &QTimer::timeout, this, &MainWindow::RefreshStatus);
    refresh_timer.start();

    job_processor_status = new QLabel(nullptr);
    job_processor_status->setText("Idle");
    job_processor_status->setMinimumSize({100, 0});
    job_processor_status->setAlignment(Qt::AlignCenter);
    ui->statusBar->addPermanentWidget(job_processor_status, 0);

    ui->statusBar->showMessage("", 500);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::TabCloseRequested);
#ifdef DEBUG
    setWindowTitle(windowTitle() + " [DEBUG]");
#endif
}

MainWindow::~MainWindow() {
    ui.reset();
}

bool MainWindow::PostInit() {
    auto mm = GetModuleManager();
    // jobProcessor = mm->QueryModule<QtShared::iJobProcessor>();

    mm->ForEachInterface<BaseDockWindowModule>([this](auto interface, auto module) {
        if (interface->IsMainMenu()) {
            interface->SetParent(this);
            ui->menuWindows->addAction(interface->GetIcon(), interface->GetDisplayName(),
                                       interface.get(), SLOT(Show()), interface->GetKeySequence());
            AddLogf(Info, "Registered DockWindow: %s", module->GetModuleName().c_str());
        }
    });

    show();
    return true;
}

bool MainWindow::Finalize() {
    // jobProcessor.reset();
    return true;
}

void MainWindow::RefreshStatus() {
    // if (jobProcessor) {
    //     auto jobs = jobProcessor->GetQueuedJobCount();
    //     if (jobs > 0) {
    //         job_processor_status->setText(("Queued Jobs: " + std::to_string(jobs)).c_str());
    //         job_processor_status->setStyleSheet("QLabel { background-color : red; }");
    //     } else {
    //         job_processor_status->setText("Idle");
    //         job_processor_status->setStyleSheet("QLabel { }");
    //     }
    // }
}

void MainWindow::DoSaveSettings(nlohmann::json &json) const {
    iWidgetSettingsProvider::DoSaveSettings(json);
    json["MainWindowSettings"] = settings;
}

void MainWindow::DoLoadSettings(const nlohmann::json &json) {
    iWidgetSettingsProvider::DoLoadSettings(json);
    try_get_json_child(json, "MainWindowSettings", settings);

    if (settings.load_last_module && !settings.recent_modules.empty()) {
        auto last = settings.recent_modules.back();
        if (std::filesystem::is_directory(last)) {
            QTimer::singleShot(500, [this, last = std::move(last)] { OpenModule(last); });
        }
    }
}

void MainWindow::NewModuleAction() {
    ReportNotImplemented();
    //     if (m_DataModule) {
    //         if (!AskForPermission()) {
    //             return;
    //         }
    //     }

    //     auto f = QFileDialog::getExistingDirectory(this, "MoonGlare", QString());

    //     if (f.isEmpty())
    //         return;

    //     if (m_DataModule)
    //         CloseModule();
    //     NewModule(f.toLatin1().constData());
}

void MainWindow::OpenModuleAction() {
    auto f = QFileDialog::getExistingDirectory(this, "MoonGlare", QString());
    if (f.isEmpty())
        return;

    auto fs = GetModuleManager()->QueryModule<Modules::FileSystemModule>();
    if (fs->IsPathOpened()) {
        ReportNotImplemented();
        return;
    }

    OpenModule(f.toStdString());
}

void MainWindow::CloseModuleAction() {
    ReportNotImplemented();
    //     if (m_DataModule) {
    //         if (!AskForPermission()) {
    //             return;
    //         }
    //         CloseModule();
    //     }
}

void MainWindow::CloseEditorAction() {
    //     if (m_DataModule) {
    //         if (!AskForPermission()) {
    //             return;
    //         }
    //         CloseModule();
    //     }

    close();
}

// void MainWindow::BuildModuleAction() {
//     BuildOptions w(this, m_DataModule, GetModuleManager());
//     if (w.exec() != QDialog::Accepted)
//         return;

//     auto settings = w.GetSettings();
//     settings.InputSettingsFile = m_DataModule->GetInputSettingsFile();
//     settings.moduleSourceLocation = m_DataModule->GetBaseDirectory();
//     settings.binLocation = std::string(QApplication::applicationDirPath().toLocal8Bit().data()) + "/";

//     auto pm = GetModuleManager()->QueryModule<MoonGlare::Module::BackgroundProcessManager>();
//     auto process = pm->CreateProcess<BuildProcess>(std::to_string(rand()), settings);
//     ForegroundProcess fp(this, GetModuleManager(), process, false);
//     fp.exec();
// }

// void MainWindow::PackModuleAction() {
//     ReportNotImplemented();
//     // BuildOptions w(this, GetModuleManager());
//     // if (w.exec() != QDialog::Accepted)
//     // return;

//     // bs.outputLocation = ui->lineEditOutput->text().toLocal8Bit().begin();
//     // bs.debugBuild = ui->checkBoxDebug->isChecked();
//     // bs.disableValidation = ui->checkBoxDisableValidation->isChecked();

//     // Editor::BuildOptions settings = w.GetSettings(
//     // settings.
//     // settings.moduleSourceLocation = m_DataModule->GetBaseDirectory();
//     // settings.binLocation = std::string(QApplication::applicationDirPath().toLocal8Bit().data()) + "/";

//     // auto pm = GetModuleManager()->QueryModule<QtShared::BackgroundProcessManager>();
//     // auto process = pm->CreateProcess<BuildProcess>(std::to_string(rand()), settings);
//     // ForegroundProcess fp(this, GetModuleManager(), process, false);
//     // fp.exec();
// }

//-----------------------------------------

void MainWindow::OpenModule(const std::string &root_directory) {
    if (!std::filesystem::is_directory(root_directory)) {
        return;
    }

    auto fs = GetModuleManager()->QueryModule<Modules::FileSystemModule>();
    if (fs->IsPathOpened()) {
        return;
    }

    fs->OpenPath(root_directory);
    settings.recent_modules.erase(
        std::remove(settings.recent_modules.begin(), settings.recent_modules.end(), root_directory),
        settings.recent_modules.end());
    settings.recent_modules.push_back(root_directory);

    AddLogf(Info, "Opened module: %s", root_directory.c_str());
    setWindowTitle(windowTitle() + " - " + root_directory.c_str());
}

// void MainWindow::CloseModule() {
//     if (!m_DataModule)
//         return;

//     m_DataModule->SaveNow();
//     m_DataModule.reset();

//     Notifications::SendProjectChanged(m_DataModule);
// }

// //-----------------------------------------

void MainWindow::showEvent(QShowEvent *event) {
    event->accept();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    auto changes_manager = GetModuleManager()->QueryModule<iChangesManager>();
    if (changes_manager && changes_manager->IsChanged()) {
        if (AskToDropChanges()) {
            changes_manager->DropChanges();
        } else {
            event->ignore();
            return;
        }
    }

    GetModuleManager()->QueryModule<RuntineModules::ApplicationSettings>()->SaveAndLock();
    event->accept();
}

// //-----------------------------------------

// void MainWindow::AddAction(std::string id, ActionVariant action, std::weak_ptr<iActionProvider> provider) {
//     auto it = actionBarItems.find(id);
//     if (it != actionBarItems.end()) {
//         __debugbreak();
//         return;
//     }

//     ActionInfo ai;
//     ai.action = action;
//     ai.provider = provider;

//     QAction *ac = ai.GetActionPtr();

//     if (ac == nullptr) {
//         ai.action = ui->mainToolBar->addSeparator();
//     } else {
//         ui->mainToolBar->addAction(ac);
//     }
//     actionBarItems[id] = ai;
// }

// void MainWindow::RemoveProvider(std::weak_ptr<iActionProvider> provider) {
//     for (auto it = actionBarItems.begin(), jt = actionBarItems.end(); it != jt;) {
//         const auto &item = it->second;
//         if (item.provider.lock() == provider.lock()) {
//             ui->mainToolBar->removeAction(item.GetActionPtr());
//             auto remove = it;
//             ++it;
//             actionBarItems.erase(remove);
//         } else {
//             ++it;
//         }
//     };
// }

// //-----------------------------------------

// iMainWindowTabsCtl
QWidget *MainWindow::GetTabParentWidget() const {
    return ui->tabWidget;
}

void MainWindow::AddTab(const std::string &id, std::shared_ptr<iTabViewBase> tabWidget) {
    QWidget *w = dynamic_cast<QWidget *>(tabWidget.get());
    if (!w) {
        AddLog(Error, "Attempt to add tab that does not inherit from QWidget");
        ErrorMessage("Tab pointer is missing QWidget class");
        return;
    }
    if (opened_tabs.find(id) == opened_tabs.end()) {
        opened_tabs[id] = tabWidget;
        tabWidget->SetTablController(
            std::dynamic_pointer_cast<iMainWindowTabsControl>(shared_from_this()));
        w->setLayout(new QHBoxLayout());
        ui->tabWidget->addTab(w, tabWidget->GetTabTitle().c_str());
        ui->tabWidget->setCurrentWidget(w);
    }
}
std::shared_ptr<iTabViewBase> MainWindow::GetTab(const std::string &id) const {
    auto it = opened_tabs.find(id);
    if (it != opened_tabs.end()) {
        return it->second;
    }
    return nullptr;
}

bool MainWindow::TabExists(const std::string &id) const {
    return opened_tabs.find(id) != opened_tabs.end();
}

void MainWindow::ActivateTab(const std::string &id) {
    auto it = opened_tabs.find(id);
    if (opened_tabs.find(id) == opened_tabs.end())
        return;
    QWidget *w = dynamic_cast<QWidget *>(it->second.get());
    if (w) {
        ui->tabWidget->setCurrentWidget(w);
    }
}

void MainWindow::ActivateTab(iTabViewBase *tab_view) {
    QWidget *w = dynamic_cast<QWidget *>(tab_view);
    if (w != nullptr) {
        auto index = ui->tabWidget->indexOf(w);
        if (index >= 0) {
            ui->tabWidget->setCurrentWidget(w);
        }
    }
}

void MainWindow::TabCloseRequested(int index) {
    auto w = ui->tabWidget->widget(index);
    if (!w)
        return;

    for (auto &item : opened_tabs) {
        QWidget *iw = dynamic_cast<QWidget *>(item.second.get());
        if (iw == w && item.second->CanClose()) {
            opened_tabs.erase(item.first);
            ui->tabWidget->removeTab(index);
            return;
        }
    }
}

void MainWindow::SetTabTitle(iTabViewBase *tab_view, const std::string &title) {
    QWidget *w = dynamic_cast<QWidget *>(tab_view);
    if (w != nullptr) {
        auto index = ui->tabWidget->indexOf(w);
        if (index >= 0) {
            ui->tabWidget->setTabText(index, QString::fromStdString(title));
        }
    }
}

void MainWindow::SetTabIcon(iTabViewBase *tab_view, QIcon icon) {
    QWidget *w = dynamic_cast<QWidget *>(tab_view);
    if (w != nullptr) {
        auto index = ui->tabWidget->indexOf(w);
        if (index >= 0) {
            ui->tabWidget->setTabIcon(index, std::move(icon));
        }
    }
}

// //-----------------------------------------

// /*
// void MainWindow::CreateFileEditor(const std::string & URI, std::shared_ptr<SharedData::FileCreatorInfo> info) {
// auto inst = info->m_DockEditor->GetInstance();
// auto editor = dynamic_cast<QtShared::iEditor*>(inst.get());
// if (!editor) {
//     AddLogf(Error, "Fatal Error!");
//     ErrorMessage("Fatal Error!");
//     return;
// }

// if (editor->Create(URI, info->m_Info)) {
//     inst->show();
// } else {
//     ErrorMessage("Failed to create file!");
//     AddLog(Error, "Failed to create file!");
// }
// }
// */

} // namespace MoonGlare::Tools::Editor::Dialogs
