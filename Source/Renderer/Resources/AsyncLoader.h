#pragma once

#include "../Configuration.Renderer.h"

#include "iAsyncLoader.h"

namespace MoonGlare::Renderer::Resources {

class AsyncLoader final : public iAsyncLoader {
    using ThisClass = AsyncLoader;
    using Conf = Configuration::Resources;
public:
    AsyncLoader(ResourceManager *Owner, iFileSystem *fs, const Configuration::RuntimeConfiguration *Configuration);
    ~AsyncLoader();

    //iAsyncLoader
    unsigned JobsPending() const override;
    void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) override;
    void QueueTask(SharedAsyncTask task) override;
    void SetObserver(SharedAsyncLoaderObserver o) override;


    void SubmitShaderLoad(ShaderResourceHandleBase handle);
private:
    bool m_CanWork = false;
    std::condition_variable m_Lock;
    std::thread m_Thread;
    iFileSystem *fileSystem;
    const Configuration::RuntimeConfiguration *m_Configuration;
    WeakAsyncLoaderObserver observer;

    struct QueueData {
        Commands::CommitCommandQueue m_ccq;
        bool m_Finished;
        uint8_t _padding[3];
        void *padding2[1];
        ResourceLoadStorage storage;

        QueueData() {
            m_ccq.m_Queue = &storage.m_Queue;
            storage.m_Queue.MemZero();
            Clear();
        }

        void Clear() {
            m_Finished = false;
            m_ccq.m_Commited = false;
            storage.m_Queue.ClearAllocation();
            storage.m_Memory.m_Allocator.Clear();
        }
    };
    static_assert(Conf::AsyncQueueCount == 2, "error!");//not implemented

    std::atomic<bool> working = false;
    bool m_QueueDirty = false;
    QueueData *m_PendingQueue;
    QueueData *m_SubmitedQueue;
    ResourceManager *m_ResourceManager;
    std::array<QueueData, Conf::AsyncQueueCount> m_QueueTable;

    enum class ProcessorResult {
        Success,
        CriticalError,
        QueueFull,
        NothingDone,
        Retry,
    };

//--
    struct ShaderLoadTask {
        ShaderResourceHandleBase m_Handle;
    };
    ProcessorResult ProcessTask(QueueData *queue, ShaderLoadTask &slt);
//--
    struct AsyncFSTask {
        std::string URI;
        SharedAsyncFileSystemRequest request;
    };
    ProcessorResult ProcessTask(QueueData *queue, AsyncFSTask &afst);
//--
    ProcessorResult ProcessTask(QueueData *queue, SharedAsyncTask &afst);

    using AnyTask = std::variant <
        ShaderLoadTask,
        AsyncFSTask,
        SharedAsyncTask
    >;

    std::list<AnyTask> m_Queue;
    mutable std::mutex m_QueueMutex;
    void QueuePush(AnyTask at);

    DeclarePerformanceCounter(JobsDone);

    void ThreadMain();
};

} //namespace MoonGlare::Renderer::Resources 
