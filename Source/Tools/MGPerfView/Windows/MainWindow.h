#pragma once

#include <qevent.h>
#include <qmainwindow.h>

#include <ToolBase/Module.h>
#include <ToolBase/Modules/iSettingsUser.h>
#include <ToolBase/UserQuestions.h>
#include <ToolBase/interfaces/ActionBarSink.h>

#include "ToolBase/Tools/PerfViewServer.h"

#include <QtCharts/QChartView>

namespace Ui {
class MainWindow;
}

namespace MoonGlare::PerfView {

class ChartManager;

class MainWindow
    : public QMainWindow,
      public iModule,
      public iSettingsUser,
      public UserQuestions {
    Q_OBJECT
public:
    MainWindow(SharedModuleManager modmgr);
    ~MainWindow();


    bool PostInit() override;
protected:
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* event);

    virtual bool DoSaveSettings(pugi::xml_node node) const override;
    virtual bool DoLoadSettings(const pugi::xml_node node) override;
private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<ChartManager> chartManager;
public slots:
protected slots:
};

}
