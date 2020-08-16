#if 0
#pragma once

#include <ToolBase/Module.h>
#include <iFileProcessor.h>

namespace MoonGlare {
namespace Editor {

class AsyncFileProcessor 
	: public QObject
    , public iModule
    , public QtShared::iJobProcessor {
	Q_OBJECT;
public:
 	AsyncFileProcessor(SharedModuleManager modmgr);
 	virtual ~AsyncFileProcessor();

    void Queue(QtShared::SharedFileProcessor processor)override;
    size_t GetQueuedJobCount() override;
signals:
	void FileProcessingFinished(QtShared::SharedFileProcessor);
protected:
    size_t jobcount = 0;
    bool m_ThreadCanWork;
	std::condition_variable m_QueueEmpty;
	std::list<QtShared::SharedFileProcessor> m_Queue;
	std::mutex m_Mutex;
	std::thread m_Thread;

	void ThreadEntry();
    void ExecuteJob(MoonGlare::QtShared::SharedFileProcessor processor);
protected slots:
	void JobFinished(QtShared::SharedFileProcessor);
};

} //namespace Editor 
} //namespace MoonGlare

#endif
