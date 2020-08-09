#include "background_process.h"

namespace MoonGlare::Tools::RuntineModules {

iBackgroundProcess::iBackgroundProcess(const std::string &id, SharedModuleManager moduleManager)
    : moduleManager(moduleManager), id(id) {
    processManager = this->moduleManager->QueryModule<BackgroundProcessManager>();
    stateText = "pending";
}

void iBackgroundProcess::Start() {
    {
        State expected = State::NotStarted;
        if (!state.compare_exchange_strong(expected, State::Starting))
            return;
    }
    SetStateText("");
    thread = std::thread([this]() {
        try {
            canRun = true;

            if (!readableName.empty()) {
                auto om = this->moduleManager->QueryModule<iOutputProvider>();
                output = om->CreateOutput(GetId(), GetReadableName());
            }

            AddLogf(Debug, "Process started %s", typeid(*this).name());
            output->PushLine("prcess started");
            state = State::InProgress;
            Run();
            SetStateText("");
            {
                State expected = State::InProgress;
                state.compare_exchange_strong(expected, State::Completed);
                expected = State::Aborting;
                state.compare_exchange_strong(expected, State::Aborted);
            }
        } catch (...) {
            progress = 1;
            state = State::Failed;
            OnFailure(std::current_exception());
        }
        canRun = false;
        output->PushLine("process finished");
        AddLogf(Debug, "Process finished %s", typeid(*this).name());
        processManager.lock()->ProcessCompleted(shared_from_this());
    });
}

void iBackgroundProcess::OnFailure(std::exception_ptr exptr) {
    try {
        std::rethrow_exception(exptr);
    } catch (const char *msg) {
        output->PushLine("Failure:");
        output->PushLine(msg);
        AddLogf(Error, "Process failure: %s", msg);
    } catch (const std::exception &e) {
        output->PushLine("Failure:");
        output->PushLine(e.what());
        AddLogf(Error, "Process failure: %s", e.what());
    } catch (...) {
        output->PushLine("Unknown process failure");
        AddLogf(Error, "Unknown process failure");
    }
}

iBackgroundProcess::~iBackgroundProcess() {
    Abort();
    if (std::this_thread::get_id() == thread.get_id()) {
        thread.detach();
    } else {
        if (thread.joinable())
            thread.join();
    }
}

iBackgroundProcess::AbortAction iBackgroundProcess::Abort() { return AbortAction::Impossible; }

void iBackgroundProcess::ExecuteSteps(const std::vector<StepInfo> &steps) {
    for (size_t i = 0; i < steps.size(); ++i) {
        const auto &step = steps[i];
        auto st = fmt::format("Executing step {} of {} ({})", i + 1, steps.size(), step.name);
        SetStateText(st);
        output->PushLine(st);
        if (step.function)
            step.function();
    }
    SetStateText("");
}

//-------------------------------------------------------------------------------------------------

BackgroundProcessManager::BackgroundProcessManager(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

BackgroundProcessManager::~BackgroundProcessManager() {}

bool BackgroundProcessManager::Finalize() {
    AbortAll();
    WaitForAll();
    return true;
}

std::list<SharedBackgroundProcess> BackgroundProcessManager::GetAllProcesses() const {
    std::lock_guard lock(mutex);
    return processes;
}

void BackgroundProcessManager::AddProcess(SharedBackgroundProcess process) {
    std::lock_guard lock(mutex);
    processes.emplace_back(std::move(process));
}

void BackgroundProcessManager::ProcessCompleted(SharedBackgroundProcess process) {
    std::lock_guard lock(mutex);
    processes.remove(process);
}

void BackgroundProcessManager::WaitForAll() {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard lock(mutex);
        if (processes.empty())
            return;
    }
}

void BackgroundProcessManager::AbortAll() {
    std::lock_guard lock(mutex);
    for (auto &itm : processes)
        itm->Abort();
}

} // namespace MoonGlare::Tools::RuntineModules
