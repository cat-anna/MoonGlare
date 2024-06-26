#include "../nfRenderer.h"
#include "ResourceManager.h"
#include "AsyncLoader.h"
#include "../Renderer.h"
#include "../RenderDevice.h"

#include "../Commands/OpenGL/TextureCommands.h"   

#include <Renderer/Resources/Shader/ShaderResource.h>

using namespace std::chrono_literals;

namespace MoonGlare::Renderer::Resources {

AsyncLoader::AsyncLoader(ResourceManager *Owner, iFileSystem *fs, const Configuration::RuntimeConfiguration *Configuration) {
    assert(Owner);
    assert(fs);
    assert(Configuration);
    m_ResourceManager = Owner;
    fileSystem = fs;
    m_Configuration = Configuration;

    m_PendingQueue = &m_QueueTable[0];
    m_SubmitedQueue = &m_QueueTable[1];
    m_SubmitedQueue->m_ccq.m_Commited = true;

    m_CanWork = true;
    m_Thread = std::thread([this]() { 
        ::OrbitLogger::ThreadInfo::SetName("RALD");
        ThreadMain();
    });
}

AsyncLoader::~AsyncLoader() {
    m_CanWork = false;
    if (m_Thread.joinable())
        m_Thread.join();
}

//---------------------------------------------------------------------------------------

void AsyncLoader::ThreadMain() {
    auto Facade = m_ResourceManager->GetRendererFacade();
    auto Device = Facade->GetDevice();

    unsigned JobCounter = 0;

    while (m_CanWork) {
        if (m_PendingQueue->m_Finished) {
            if (m_SubmitedQueue->m_ccq.m_Commited == false) {
                std::this_thread::sleep_for(1ms);
                continue;
            }
            AddLogf(Performance, "Flushing queue to device. Submitted jobs: %u", JobCounter);
            JobCounter = 0;
            Device->CommitControlCommandQueue(&m_PendingQueue->m_ccq);
            std::swap(m_PendingQueue, m_SubmitedQueue);
            m_PendingQueue->Clear();
            m_QueueDirty = false;
            continue;
        }
        else {
            ProcessorResult result = ProcessorResult::NothingDone;

            AnyTask at;
            bool have = false;
            {
                LOCK_MUTEX(m_QueueMutex);
                if (!m_Queue.empty()) {
                    at.swap(m_Queue.front());
                    m_Queue.pop_front();
                    have = true;
                    AddLogf(Performance, "fetched job remain:%d", m_Queue.size());
                }
            }

            if (have) {
                result = std::visit([this](auto& item) {
                    return ProcessTask(m_PendingQueue, item);
                }, at);

                ++JobCounter;
                IncrementPerformanceCounter(JobsDone);
            }

            switch (result) {
            case ProcessorResult::Success:
                m_QueueDirty = true;
                break;
            case ProcessorResult::Retry:
                DebugLogf(Info, "Retrying job");
                QueuePush(std::move(at));
                break;
            case ProcessorResult::CriticalError:
                //not handled here
                DebugLogf(Error, "Error during processing task!");
                break;
            case ProcessorResult::NothingDone:
                if (!m_QueueDirty) {
                    std::mutex mutex;
                    std::unique_lock<std::mutex> lock(mutex);
                    m_Lock.wait_for(lock, 100ms);

                    if (LOCK_MUTEX(m_QueueMutex); !m_Queue.empty())
                        break;

                    bool v = true;
                    if (working.compare_exchange_strong(v, false) && v == true) {
                        localJobCount = 0;
                        auto ob = observer.lock();
                        if (ob)
                            ob->OnFinished(this);
                    }

                    break;
                }
                //[[fallthrough]]
            case ProcessorResult::QueueFull:
                if (have) {
                    QueuePush(std::move(at));
                }
                if (m_QueueDirty) {
                    m_PendingQueue->m_Finished = true;
                }
                else {
                    AddLogf(Error, "Queue full and not dirty!");
                }
                break;
            default:
                LogInvalidEnum(result);
                break;
            }

        }
    }
}

void AsyncLoader::QueuePush(AnyTask at) {
    LOCK_MUTEX(m_QueueMutex);
    m_Queue.emplace_back(std::move(at));
    AddLogf(Performance, "Queued Job cnt:%d", m_Queue.size());
    bool v = false;
    ++localJobCount;
    if (working.compare_exchange_strong(v, true) && v == false) {
        auto ob = observer.lock();
        if (ob)
            ob->OnStarted(this);
    }
    m_Lock.notify_one();
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void AsyncLoader::SubmitShaderLoad(ShaderResourceHandleBase handle) {
    QueuePush(ShaderLoadTask{ handle });
    m_Lock.notify_one();
}

AsyncLoader::ProcessorResult AsyncLoader::ProcessTask(QueueData *queue, ShaderLoadTask &slt) {
    auto &shres = m_ResourceManager->GetShaderResource();
    bool success = shres.GenerateReload(queue->storage.m_Queue, queue->storage.m_Memory.m_Allocator, slt.m_Handle);

    if (success) {
        return ProcessorResult::Success;
    }
    else {               
        return ProcessorResult::CriticalError;
    }
}

//---------------------------------------------------------------------------------------

void AsyncLoader::QueueTask(SharedAsyncTask task) {
    QueuePush(std::move(task));
}

AsyncLoader::ProcessorResult AsyncLoader::ProcessTask(QueueData *queue, SharedAsyncTask &task) {
    try {
        task->Do(queue->storage);
    }
    catch (BaseAsyncTask::RetryLater) {
        return ProcessorResult::Retry;
    }
    catch (const BaseAsyncTask::NotEnoughStorage &nes) {
        AddLog(Performance, fmt::format("Async FS task reported not enough storage. Used: {}/{} kbytes; required: {} kbytes",
            queue->storage.m_Memory.m_Allocator.Allocated() / 1024.0f, Conf::QueueMemory / 1024.0f, nes.requiredSpace / 1024.0f));
        if (nes.requiredSpace < Conf::QueueMemory) {
            //retry job later
            //QueuePush(task);
            return ProcessorResult::QueueFull;
        }
    }
    return ProcessorResult::Success;
}

//---------------------------------------------------------------------------------------

void AsyncLoader::QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) {
    AsyncFSTask<SharedAsyncFileSystemRequest> task;
    task.URI = std::move(URI);
    task.request = std::move(handler);
    task.request->OnTaskQueued(this);
    QueuePush(std::move(task));
}

AsyncLoader::ProcessorResult AsyncLoader::ProcessTask(QueueData *queue, AsyncFSTask<SharedAsyncFileSystemRequest> &afst) {
    StarVFS::ByteTable bt;
    if (!fileSystem->OpenFile(bt, afst.URI)) {
        AddLogf(Error, "Cannot load file %s", afst.URI.c_str());
        return ProcessorResult::CriticalError;
    }

    try {
        afst.request->OnFileReady(afst.URI, bt, queue->storage);
    }
    catch (const iAsyncFileSystemRequest::NotEnoughStorage &nes) {
        AddLog(Performance, fmt::format("Async FS task reported not enough storage. Used: {}/{} kbytes; required: {} kbytes",
            queue->storage.m_Memory.m_Allocator.Allocated() / 1024.0f, Conf::QueueMemory / 1024.0f, nes.requiredSpace / 1024.0f));
        if (nes.requiredSpace < Conf::QueueMemory) {
            //retry job later
            //QueuePush(afst);
            return ProcessorResult::QueueFull;
        }
    }
    return ProcessorResult::Success;
}

//---------------------------------------------------------------------------------------

AsyncLoader::JobStatus AsyncLoader::GetJobStatus() const {
    JobStatus js;
    LOCK_MUTEX(m_QueueMutex);
    js.localJobCount = localJobCount;
    js.pendingJobs = m_Queue.size();
    return js;
}            
                                                         
void AsyncLoader::SetObserver(MoonGlare::Resources::SharedAsyncLoaderObserver f) {
    //TODO: possible race condition
    observer = f;
}

void AsyncLoader::QueueRequest(std::string URI, MoonGlare::Resources::SharedAsyncFileSystemRequest handler) {
    AsyncFSTask<MoonGlare::Resources::SharedAsyncFileSystemRequest> task;
    task.URI = std::move(URI);
    task.request = std::move(handler);
    task.request->OnTaskQueued(this);
    QueuePush(std::move(task));
}

void AsyncLoader::QueueTask(MoonGlare::Resources::SharedAsyncTask task) {
    QueuePush(std::move(task));
}

AsyncLoader::ProcessorResult AsyncLoader::ProcessTask(QueueData *queue, AsyncFSTask<MoonGlare::Resources::SharedAsyncFileSystemRequest> &afst) {
    StarVFS::ByteTable bt;
    if (!fileSystem->OpenFile(bt, afst.URI)) {
        AddLogf(Error, "Cannot load file %s", afst.URI.c_str());
        return ProcessorResult::CriticalError;
    }

    try {
        afst.request->OnFileReady(afst.URI, bt);
    }
    catch (MoonGlare::Resources::RetryTaskLaterException) {
        return ProcessorResult::Retry;
    }
    return ProcessorResult::Success;
}

AsyncLoader::ProcessorResult AsyncLoader::ProcessTask(QueueData *queue, MoonGlare::Resources::SharedAsyncTask &task) {
    try {
        task->DoWork();
    }
    catch (MoonGlare::Resources::RetryTaskLaterException) {
        return ProcessorResult::Retry;
    }
    return ProcessorResult::Success;
}

} //namespace MoonGlare::Renderer::Resources 
 