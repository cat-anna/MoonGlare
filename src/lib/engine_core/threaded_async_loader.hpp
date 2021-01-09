#pragma once

#include <async_loader.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <readonly_file_system.h>
#include <synchronized_task_queue.hpp>
#include <thread>
#include <variant>

namespace MoonGlare {

class ThreadedAsyncLoader final : public iAsyncLoader {
public:
    ThreadedAsyncLoader(std::shared_ptr<iReadOnlyFileSystem> filesystem);
    ~ThreadedAsyncLoader() override;

    // iAsyncLoader
    // void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) override;
    // void QueueTask(SharedAsyncTask task) override;

    // iAsyncLoader
    void SetObserver(SharedAsyncLoaderObserver o) override;
    void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) override;
    void QueueTask(SharedAsyncTask task) override;
    AsyncLoaderStatus GetStatus() const override;

    // void SubmitShaderLoad(ShaderResourceHandleBase handle);

private:
    using AnyTask = std::variant<SharedAsyncTask>;

    std::thread work_thread;
    std::atomic<bool> can_work{false};
    std::atomic<bool> working{false};

    std::shared_ptr<iReadOnlyFileSystem> filesystem;
    WeakAsyncLoaderObserver observer;

    std::atomic<size_t> processed_jobs{0};
    std::atomic<size_t> local_job_count{0};

    std::condition_variable worker_condition_variable;
    SynchronizedTaskQueue<AnyTask> queue;

    void QueuePush(AnyTask at);

    // const Configuration::RuntimeConfiguration *m_Configuration;

    // struct QueueData {
    //     Commands::CommitCommandQueue m_ccq;
    //     bool m_Finished;
    //     uint8_t _padding[3];
    //     void *padding2[1];
    //     ResourceLoadStorage storage;

    //     QueueData() {
    //         m_ccq.m_Queue = &storage.m_Queue;
    //         storage.m_Queue.MemZero();
    //         Clear();
    //     }

    //     void Clear() {
    //         m_Finished = false;
    //         m_ccq.m_Commited = false;
    //         storage.m_Queue.ClearAllocation();
    //         storage.m_Memory.m_Allocator.Clear();
    //     }
    // };
    // static_assert(Conf::AsyncQueueCount == 2, "error!"); // not implemented

    // bool m_QueueDirty = false;
    // QueueData *m_PendingQueue;
    // QueueData *m_SubmitedQueue;
    // ResourceManager *m_ResourceManager;
    // std::array<QueueData, Conf::AsyncQueueCount> m_QueueTable;

    // enum class ProcessorResult {
    //     Success,
    //     CriticalError,
    //     QueueFull,
    //     NothingDone,
    //     Retry,
    // };

    // template <typename TASK> struct AsyncFSTask {
    //     std::string URI;
    //     TASK request;
    // };

    // struct ShaderLoadTask {
    //     ShaderResourceHandleBase m_Handle;
    // };
    // ProcessorResult ProcessTask(QueueData *queue, ShaderLoadTask &slt);

    // ProcessorResult ProcessTask(QueueData *queue, AsyncFSTask<SharedAsyncFileSystemRequest> &afst);
    // ProcessorResult ProcessTask(QueueData *queue, SharedAsyncTask &afst);
    // ProcessorResult ProcessTask(QueueData *queue,
    //                             AsyncFSTask<MoonGlare::Resources::SharedAsyncFileSystemRequest> &afst);
    // ProcessorResult ProcessTask(QueueData *queue, MoonGlare::Resources::SharedAsyncTask &afst);

    // using AnyTask = std::variant<ShaderLoadTask, AsyncFSTask<SharedAsyncFileSystemRequest>, SharedAsyncTask,
    //                              AsyncFSTask<MoonGlare::Resources::SharedAsyncFileSystemRequest>,
    //                              MoonGlare::Resources::SharedAsyncTask>;

    // DeclarePerformanceCounter(JobsDone);

    void ThreadMain();
};

} // namespace MoonGlare
