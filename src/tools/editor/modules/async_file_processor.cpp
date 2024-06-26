#if 0

#include PCH_HEADER
#include "AsyncFileProcessor.h"

namespace MoonGlare {
namespace Editor {

ModuleClassRegister::Register<AsyncFileProcessor> AsyncFileProcessorReg("AsyncFileProcessor");

//----------------------------------------------------------------------------------

AsyncFileProcessor::AsyncFileProcessor(SharedModuleManager modmgr)
        : iModule(std::move(modmgr)) {
    m_ThreadCanWork = true;
    m_QueueEmpty.notify_one();
    m_Thread = std::thread([this]() { ThreadEntry(); });

    connect(this, &AsyncFileProcessor::FileProcessingFinished, this, &AsyncFileProcessor::JobFinished, Qt::QueuedConnection);
}

AsyncFileProcessor::~AsyncFileProcessor() {
    m_ThreadCanWork = false;
    m_Thread.join();
}

size_t AsyncFileProcessor::GetQueuedJobCount() {
    LOCK_MUTEX(m_Mutex);
    return m_Queue.size();
}

void AsyncFileProcessor::JobFinished(QtShared::SharedFileProcessor p) {
    p->HandlePostProcess();
}

void AsyncFileProcessor::Queue(QtShared::SharedFileProcessor p) {
    LOCK_MUTEX(m_Mutex);
    m_Queue.emplace_back(std::move(p));
    //AddLogf(Info, "Queued job, count: %u", m_Queue.size());
    m_QueueEmpty.notify_one();
}

void AsyncFileProcessor::ThreadEntry() {
    OrbitLogger::ThreadInfo::SetName("AFPT");
    while (m_ThreadCanWork) {
        QtShared::SharedFileProcessor processor;
        {
            LOCK_MUTEX(m_Mutex);
            if (!m_Queue.empty()) {
                processor = m_Queue.front();
                m_Queue.pop_front();
            }
        }

        if (processor) {
            auto fence = processor->GetFence();
            if (!fence) {
                ExecuteJob(std::move(processor));
                continue;
            }
            std::unique_lock<QtShared::iJobFence> fenceLock(*fence, std::defer_lock);
            if (!fenceLock.try_lock()) {
                LOCK_MUTEX(m_Mutex);
                m_Queue.push_back(std::move(processor));
            } else {
                ExecuteJob(std::move(processor));
            }
            continue;
        }

        std::mutex m;
        std::unique_lock<std::mutex> lk(m);
        m_QueueEmpty.wait_for(lk, std::chrono::seconds(1));
    }
}

void AsyncFileProcessor::ExecuteJob(MoonGlare::QtShared::SharedFileProcessor processor) {
    ++jobcount;
    AddLogf(Info, "Fetched job #%u", jobcount);
    auto result = processor->ProcessFile();
    AddLogf(Info, "Job #%u result: %u", jobcount, static_cast<unsigned>(result));

    if (result == QtShared::iFileProcessor::ProcessResult::RetryLater) {
        LOCK_MUTEX(m_Mutex);
        m_Queue.emplace_back(processor);
    }
    else {
        emit FileProcessingFinished(processor);
    }
}

} //namespace Editor 
} //namespace MoonGlare

#endif
