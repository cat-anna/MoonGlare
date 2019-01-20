
#include <qlabel.h>
#include <qobject.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../ChartManager.h"
#include "../ChartView.h"

namespace MoonGlare::PerfView {

MoonGlare::ModuleClassRgister::Register<MainWindow> MainWindowReg("MainWindow");

MainWindow::MainWindow(SharedModuleManager modmgr)
    : QMainWindow(nullptr), iModule(std::move(modmgr))
{
    SetSettingID("MainWindow");

    ui = std::make_unique<Ui::MainWindow>();
    ui->setupUi(this);

    chartManager = std::make_unique<ChartManager>(ui->scrollAreaContent, ui->verticalLayout_2);
}

MainWindow::~MainWindow() {
    chartManager.reset();
    ui.reset();
}

bool MainWindow::PostInit() { 
    show();
    return true;
}

void MainWindow::showEvent(QShowEvent* event)
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->accept();
}

bool MainWindow::DoSaveSettings(pugi::xml_node node) const {
    SaveGeometry(node, this, "Qt:Geometry");
    SaveState(node, this, "Qt:State");
    return true;
}

bool MainWindow::DoLoadSettings(const pugi::xml_node node) {
    LoadGeometry(node, this, "Qt:Geometry");
    LoadState(node, this, "Qt:State");

    return true;
}

}
