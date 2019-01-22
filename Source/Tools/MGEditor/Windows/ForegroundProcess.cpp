#include PCH_HEADER
#include "ForegroundProcess.h"
#include "ui_ForegroundProcess.h"
#include <qabstractbutton.h>
#include <qobject.h>

#include <ToolBase/UserQuestions.h>

namespace MoonGlare {
namespace Editor {

ForegroundProcess::ForegroundProcess(QWidget *parent, SharedModuleManager ModuleManager, SharedBackgroundProcess Process, bool AllowGotoBackground)
    : SubDialog(parent, ModuleManager), process(Process)
{
    ui = std::make_unique<Ui::ForegroundProcess>();
    ui->setupUi(this);

    connect(ui->pushButtonOk, &QAbstractButton::clicked, this, &ForegroundProcess::ButtonOk);
    connect(ui->pushButtonHide, &QAbstractButton::clicked, this, &ForegroundProcess::ButtonHide);
    connect(ui->pushButtonAbort, &QAbstractButton::clicked, this, &ForegroundProcess::ButtonAbort);

    ui->pushButtonHide->setVisible(AllowGotoBackground);
    ui->pushButtonOk->setEnabled(false);

    refreshTimer = std::make_unique<QTimer>(this);
    connect(refreshTimer.get(), &QTimer::timeout, [this]() { Refresh(true); });
    refreshTimer->setSingleShot(false);
    refreshTimer->setInterval(500);
    refreshTimer->start();
    Refresh(false);
}

ForegroundProcess::~ForegroundProcess() {
    refreshTimer.reset();
    ui.reset();
}

void ForegroundProcess::Refresh(bool CanStart) {
    std::optional<float> progress = std::nullopt;
    std::string stateText;

    auto Finished = [this] {
        ui->pushButtonOk->setEnabled(true);
        refreshTimer->stop();
        ui->pushButtonAbort->setEnabled(false);
    };

    using State = iBackgroundProcess::State;
    switch (process->GetState())
    {
    case State::NotStarted:
        stateText = "Not started";
        if(CanStart)
            process->Start();
        break;
    case State::Starting:
        stateText = "Starting";
        break;
    case State::InProgress:
        progress = process->GetProgress();
        stateText = "In progress";
        break;
    case State::Aborting:
        stateText = "Aborting";
        break;
    case State::Aborted:
        stateText = "Aborted";
        Finished();
        break;
    case State::Completed:
        stateText = "Completed";
        progress = 1.0f;
        Finished();
        break;
    case State::Failed:
        stateText = "Failed";
        progress = 1.0f;
        Finished();
        break;
    default:
        LogInvalidEnum(process->GetState());
        return;
    }

    if (progress.has_value() && *progress >= 0.0f) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(static_cast<int>(*progress * 100));
    } else {
        ui->progressBar->setMaximum(0);
        ui->progressBar->setValue(0);
    }

    ui->labelProcess->setText((std::string("Task in progress: ") + process->GetReadableName()).c_str());

    auto processStateText = process->GetStateText();
    if(!processStateText.empty())
        ui->labelState->setText((stateText + " -> " + processStateText).c_str());
    else
        ui->labelState->setText(stateText.c_str());
}

void ForegroundProcess::ButtonAbort() {
    if (!AskForPermission())
        return;

    using AbortAction = iBackgroundProcess::AbortAction;
    if (process->Abort() == AbortAction::Impossible)
        ErrorMessage("Process refused to abort");
}

void ForegroundProcess::ButtonHide() {
    accept();
    close();
}

void ForegroundProcess::ButtonOk() {
    accept();
    close();
}

//----------------------------------------------------------------------------------

} //namespace Editor
} //namespace MoonGlare
