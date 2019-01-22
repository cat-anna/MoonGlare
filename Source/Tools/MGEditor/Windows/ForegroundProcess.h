#pragma once

#include <SubDialog.h>
#include <ToolBase/Modules/iBackgroundProcess.h>
#include <ToolBase/UserQuestions.h>

namespace Ui { class ForegroundProcess; }

namespace MoonGlare {
namespace Editor {

class ForegroundProcess
    : public QtShared::SubDialog
    , public UserQuestions
{
    Q_OBJECT
public:
    ForegroundProcess(QWidget *parent, SharedModuleManager ModuleManager, SharedBackgroundProcess Process, bool AllowGotoBackground);
    ~ForegroundProcess();
private:
    std::unique_ptr<Ui::ForegroundProcess> ui;
    SharedBackgroundProcess process;
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

