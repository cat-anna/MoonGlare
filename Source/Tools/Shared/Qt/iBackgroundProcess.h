#pragma once

#include <optional>
#include <memory>
#include <thread>
#include <atomic>
#include "Module.h"
#include "iOutput.h"

namespace MoonGlare {
namespace QtShared {

class BackgroundProcessManager;
using SharedBackgroundProcessManager = std::shared_ptr<BackgroundProcessManager>;
using WeakBackgroundProcessManager = std::weak_ptr<BackgroundProcessManager>;

class iBackgroundProcess;
using SharedBackgroundProcess = std::shared_ptr<iBackgroundProcess>;

class iBackgroundProcess : public std::enable_shared_from_this<iBackgroundProcess> {
public:
    iBackgroundProcess(const std::string &id, SharedModuleManager moduleManager);
    virtual ~iBackgroundProcess();

    enum class Action{ InProgress, Impossible, };
    virtual Action Abort();
    void Start();

    float GetProgress() const { return progress; }
    const std::string& GetStateText() const { return stateText; }
    const std::string& GetId() const { return id; }
    const std::string& GetReadableName() const { return readableName; }
protected:
    SharedModuleManager moduleManager;
    WeakBackgroundProcessManager processManager;
    SharedOutput output;
    std::string id, readableName, stateText;
    float progress = -1.0f;
    std::thread thread;
    bool canRun = false;

    virtual void Run() = 0;
    virtual void OnFailure(std::exception_ptr exptr);
private:
    std::atomic<bool> started = false;
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

} //namespace QtShared
} //namespace MoonGlare
