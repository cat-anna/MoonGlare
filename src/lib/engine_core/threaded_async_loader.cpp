#include "threaded_async_loader.hpp"
#include <chrono>
#include <orbit_logger.h>

// #include "../nfRenderer.h"
// #include "ResourceManager.h"
// #include "ThreadedAsyncLoader.h"
// #include "../Renderer.h"
// #include "../RenderDevice.h"
// #include "../Commands/OpenGL/TextureCommands.h"
// #include <Renderer/Resources/Shader/ShaderResource.h>

using namespace std::chrono_literals;

namespace MoonGlare {

ThreadedAsyncLoader::ThreadedAsyncLoader(std::shared_ptr<iReadOnlyFileSystem> filesystem)
    : filesystem(std::move(filesystem)) {
    // assert(Owner);
    // assert(fs);
    // assert(Configuration);
    // m_ResourceManager = Owner;
    // m_Configuration = Configuration;

    // m_PendingQueue = &m_QueueTable[0];
    // m_SubmitedQueue = &m_QueueTable[1];
    // m_SubmitedQueue->m_ccq.m_Commited = true;

    can_work = true;
    work_thread = std::thread([this]() { ThreadMain(); });
}

ThreadedAsyncLoader::~ThreadedAsyncLoader() {
    can_work = false;
    worker_condition_variable.notify_all();
    if (work_thread.joinable()) {
        work_thread.join();
    }
}

//---------------------------------------------------------------------------------------

void ThreadedAsyncLoader::ThreadMain() {
    ::OrbitLogger::ThreadInfo::SetName("RALD");
    while (can_work.load()) {
        // if (m_PendingQueue->m_Finished) {
        //     if (m_SubmitedQueue->m_ccq.m_Commited == false) {
        //         std::this_thread::sleep_for(1ms);
        //         continue;
        //     }
        //     AddLogf(Performance, "Flushing queue to device. Submitted jobs: %u", JobCounter);
        //     JobCounter = 0;
        //     Device->CommitControlCommandQueue(&m_PendingQueue->m_ccq);
        //     std::swap(m_PendingQueue, m_SubmitedQueue);
        //     m_PendingQueue->Clear();
        //     m_QueueDirty = false;
        //     continue;
        // } else {
        // ProcessorResult result = ProcessorResult::NothingDone;

        AnyTask at;
        bool have = queue.Pop(at);
        if (have) {
            AddLogf(Performance, "fetched job remain:%d", queue.QueuedCount());
        }

        if (have) {
            // result = std::visit([this](auto &item) { return ProcessTask(m_PendingQueue, item); }, at);
            // ++JobCounter;
            // IncrementPerformanceCounter(JobsDone);
        } else {
            {
                std::mutex mutex;
                std::unique_lock<std::mutex> lock(mutex);
                worker_condition_variable.wait_for(lock, 100ms);
            }

            if (queue.Empty()) {
                continue;
            }

            bool v = true;
            if (working.compare_exchange_strong(v, false) && v == true) {
                local_job_count = 0;
                auto ob = observer.lock();
                if (ob) {
                    ob->OnFinished(this);
                }
            }

            // break;
        }

        //     switch (result) {
        //     case ProcessorResult::Success:
        //         m_QueueDirty = true;
        //         break;
        //     case ProcessorResult::Retry:
        //         DebugLogf(Info, "Retrying job");
        //         QueuePush(std::move(at));
        //         break;
        //     case ProcessorResult::CriticalError:
        //         //not handled here
        //         DebugLogf(Error, "Error during processing task!");
        //         break;
        //     case ProcessorResult::NothingDone:
        //         if (!m_QueueDirty) {
        //             std::mutex mutex;
        //             std::unique_lock<std::mutex> lock(mutex);
        //             m_Lock.wait_for(lock, 100ms);
        //             if (LOCK_MUTEX(m_QueueMutex); !m_Queue.empty())
        //                 break;
        //             bool v = true;
        //             if (working.compare_exchange_strong(v, false) && v == true) {
        //                 localJobCount = 0;
        //                 auto ob = observer.lock();
        //                 if (ob)
        //                     ob->OnFinished(this);
        //             }
        //             break;
        //         }
        //         //[[fallthrough]]
        //     case ProcessorResult::QueueFull:
        //         if (have) {
        //             QueuePush(std::move(at));
        //         }
        //         if (m_QueueDirty) {
        //             m_PendingQueue->m_Finished = true;
        //         } else {
        //             AddLogf(Error, "Queue full and not dirty!");
        //         }
        //         break;
        //     default:
        //         LogInvalidEnum(result);
        //         break;
        //     }
        // }
    }
}

void ThreadedAsyncLoader::QueuePush(AnyTask at) {
    queue.Push(std::move(at));
    AddLogf(Performance, "Queued Job cnt:%d", queue.QueuedCount());

    ++local_job_count;
    bool v = false;
    if (working.compare_exchange_strong(v, true) && v == false) {
        auto ob = observer.lock();
        if (ob) {
            ob->OnStarted(this);
        }
    }
    worker_condition_variable.notify_one();
}

//---------------------------------------------------------------------------------------

// void ThreadedAsyncLoader::SubmitShaderLoad(ShaderResourceHandleBase handle) {
//     QueuePush(ShaderLoadTask{handle});
//     m_Lock.notify_one();
// }

// ThreadedAsyncLoader::ProcessorResult ThreadedAsyncLoader::ProcessTask(QueueData *queue, ShaderLoadTask &slt) {
//     auto &shres = m_ResourceManager->GetShaderResource();
//     bool success = shres.GenerateReload(queue->storage.m_Queue, queue->storage.m_Memory.m_Allocator, slt.m_Handle);

//     if (success) {
//         return ProcessorResult::Success;
//     } else {
//         return ProcessorResult::CriticalError;
//     }
// }

//---------------------------------------------------------------------------------------

// void ThreadedAsyncLoader::QueueTask(SharedAsyncTask task) {
//     QueuePush(std::move(task));
// }

// ThreadedAsyncLoader::ProcessorResult ThreadedAsyncLoader::ProcessTask(QueueData *queue, SharedAsyncTask &task) {
//     try {
//         task->Do(queue->storage);
//     } catch (BaseAsyncTask::RetryLater) {
//         return ProcessorResult::Retry;
//     } catch (const BaseAsyncTask::NotEnoughStorage &nes) {
//         AddLog(Performance,
//                fmt::format("Async FS task reported not enough storage. Used: {}/{} kbytes; required: {} kbytes",
//                            queue->storage.m_Memory.m_Allocator.Allocated() / 1024.0f, Conf::QueueMemory / 1024.0f,
//                            nes.requiredSpace / 1024.0f));
//         if (nes.requiredSpace < Conf::QueueMemory) {
//             //retry job later
//             //QueuePush(task);
//             return ProcessorResult::QueueFull;
//         }
//     }
//     return ProcessorResult::Success;
// }

//---------------------------------------------------------------------------------------

// void ThreadedAsyncLoader::QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) {
//     AsyncFSTask<SharedAsyncFileSystemRequest> task;
//     task.URI = std::move(URI);
//     task.request = std::move(handler);
//     task.request->OnTaskQueued(this);
//     QueuePush(std::move(task));
// }

// ThreadedAsyncLoader::ProcessorResult ThreadedAsyncLoader::ProcessTask(QueueData *queue,
//                                                                       AsyncFSTask<SharedAsyncFileSystemRequest> &afst) {
//     StarVFS::ByteTable bt;
//     if (!fileSystem->OpenFile(bt, afst.URI)) {
//         AddLogf(Error, "Cannot load file %s", afst.URI.c_str());
//         return ProcessorResult::CriticalError;
//     }

//     try {
//         afst.request->OnFileReady(afst.URI, bt, queue->storage);
//     } catch (const iAsyncFileSystemRequest::NotEnoughStorage &nes) {
//         AddLog(Performance,
//                fmt::format("Async FS task reported not enough storage. Used: {}/{} kbytes; required: {} kbytes",
//                            queue->storage.m_Memory.m_Allocator.Allocated() / 1024.0f, Conf::QueueMemory / 1024.0f,
//                            nes.requiredSpace / 1024.0f));
//         if (nes.requiredSpace < Conf::QueueMemory) {
//             //retry job later
//             //QueuePush(afst);
//             return ProcessorResult::QueueFull;
//         }
//     }
//     return ProcessorResult::Success;
// }

//---------------------------------------------------------------------------------------

ThreadedAsyncLoader::AsyncLoaderStatus ThreadedAsyncLoader::GetStatus() const {
    AsyncLoaderStatus als;
    als.pending_jobs = queue.QueuedCount();
    als.local_job_count = local_job_count.load();
    als.processed_jobs = processed_jobs.load();
    return als;
}

void ThreadedAsyncLoader::SetObserver(SharedAsyncLoaderObserver f) {
    //TODO: possible race condition
    observer = f;
}

void ThreadedAsyncLoader::QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) {
    // AsyncFSTask<MoonGlare::Resources::SharedAsyncFileSystemRequest> task;
    // task.URI = std::move(URI);
    // task.request = std::move(handler);
    // task.request->OnTaskQueued(this);
    // QueuePush(std::move(task));
}

void ThreadedAsyncLoader::QueueTask(SharedAsyncTask task) {
    QueuePush(std::move(task));
}

// ThreadedAsyncLoader::ProcessorResult
// ThreadedAsyncLoader::ProcessTask(QueueData *queue,
//                                  AsyncFSTask<MoonGlare::Resources::SharedAsyncFileSystemRequest> &afst) {
//     StarVFS::ByteTable bt;
//     if (!fileSystem->OpenFile(bt, afst.URI)) {
//         AddLogf(Error, "Cannot load file %s", afst.URI.c_str());
//         return ProcessorResult::CriticalError;
//     }

//     try {
//         afst.request->OnFileReady(afst.URI, bt);
//     } catch (MoonGlare::Resources::RetryTaskLaterException) {
//         return ProcessorResult::Retry;
//     }
//     return ProcessorResult::Success;
// }

// ThreadedAsyncLoader::ProcessorResult ThreadedAsyncLoader::ProcessTask(QueueData *queue,
//                                                                       MoonGlare::Resources::SharedAsyncTask &task) {
//     try {
//         task->DoWork();
//     } catch (MoonGlare::Resources::RetryTaskLaterException) {
//         return ProcessorResult::Retry;
//     }
//     return ProcessorResult::Success;
// }

} // namespace MoonGlare
