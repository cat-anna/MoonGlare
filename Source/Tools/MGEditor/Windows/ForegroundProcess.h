#pragma once

#include <qtUtils.h>
#include <SubDialog.h>
#include <iBackgroundProcess.h>

namespace Ui { class ForegroundProcess; }

namespace MoonGlare {
namespace Editor {

class ForegroundProcess
    : public QtShared::SubDialog
    , public QtShared::UserQuestions
{
    Q_OBJECT
public:
    ForegroundProcess(QWidget *parent, SharedModuleManager ModuleManager, QtShared::SharedBackgroundProcess Process, bool AllowGotoBackground);
    ~ForegroundProcess();
private:
    std::unique_ptr<Ui::ForegroundProcess> ui;
    QtShared::SharedBackgroundProcess process;
    std::unique_ptr<QTimer> refreshTimer;
    bool FirstRefresh = true;
    void Refresh(bool CanStart);
protected slots:
    void ButtonAbort();
    void ButtonHide();
    void ButtonOk();
};

} //namespace Editor
} //namespace MoonGlare

