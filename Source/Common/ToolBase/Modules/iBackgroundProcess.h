#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include <ToolBase/Module.h>
#include <ToolBase/iOutput.h>

namespace MoonGlare {

class BackgroundProcessManager;
using SharedBackgroundProcessManager = std::shared_ptr<BackgroundProcessManager>;
using WeakBackgroundProcessManager = std::weak_ptr<BackgroundProcessManager>;

class iBackgroundProcess;
using SharedBackgroundProcess = std::shared_ptr<iBackgroundProcess>;

class iBackgroundProcess : public std::enable_shared_from_this<iBackgroundProcess> {
public:
    iBackgroundProcess(const std::string &id, SharedModuleManager moduleManager);
    virtual ~iBackgroundProcess();

    enum class State { NotStarted, Starting, InProgress, Aborting, Aborted, Completed, Failed, };

    enum class AbortAction{ InProgress, Impossible, };
    virtual AbortAction Abort();
    void Start();

    float GetProgress() const { return progress; }
    const std::string& GetStateText() const { return stateText; }
    const std::string& GetId() const { return id; }
    const std::string& GetReadableName() const { return readableName; }
    State GetState() const { return state; }
protected:
    SharedModuleManager moduleManager;
    WeakBackgroundProcessManager processManager;
    SharedOutput output;
    std::string id, readableName; //FIXME: possible race on GetXXX string functions!
    float progress = -1.0f;
    std::thread thread;
    bool canRun = false;
    std::atomic<State> state = State::NotStarted;

    virtual void Run() = 0;
    virtual void OnFailure(std::exception_ptr exptr);

    void SetStateText(std::string text) { stateText.swap(text); }

    struct StepInfo {
        const std::string name;
        std::function<void()> function;
    };
    void ExecuteSteps(const std::vector<StepInfo> &steps);
private:
    std::atomic<bool> started = false;
    std::string stateText;
};

class BackgroundProcessManager : public iModule {
public:
    BackgroundProcessManager(SharedModuleManager modmgr);
    ~BackgroundProcessManager();

    std::list<SharedBackgroundProcess> GetAllProcesses() const;

    void AddProcess(SharedBackgroundProcess process);
    void ProcessCompleted(SharedBackgroundProcess process);

    template<typename T, typename ... ARGS>
    std::shared_ptr<T> CreateProcess(const std::string &id, ARGS ... args) {
        auto proc = std::make_shared<T>(id, GetModuleManager(), std::forward<ARGS>(args)...);
        AddProcess(proc);
        return proc;
    }

    void AbortAll();
    void WaitForAll();

    bool Finalize() override;
private:
    std::list<SharedBackgroundProcess> processes;
    mutable std::recursive_mutex mutex;
};

} //namespace MoonGlare
