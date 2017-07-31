#pragma once

#include "Configuration.Renderer.h"
#include "Commands/CommandQueue.h"

namespace MoonGlare::Renderer {


class iAsyncFileSystemRequest;
using SharedAsyncFileSystemRequest = std::shared_ptr<iAsyncFileSystemRequest>;

class iAsyncTask;
using SharedAsyncTask = std::shared_ptr<iAsyncTask>;

class iAsyncLoaderObserver {
public:
    virtual ~iAsyncLoaderObserver() {};

    virtual void OnFinished() {};
    virtual void OnStarted() {};
};

using SharedAsyncLoaderObserver = std::shared_ptr<iAsyncLoaderObserver>;
using WeakAsyncLoaderObserver = std::weak_ptr<iAsyncLoaderObserver>;

class iAsyncLoader {
public:
    virtual ~iAsyncLoader() {};

    virtual bool AnyJobPending() = 0;
    virtual bool AllResoucecsLoaded() = 0;
    virtual void SetObserver(SharedAsyncLoaderObserver) = 0;

    virtual void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) = 0;
    virtual void QueueTask(SharedAsyncTask task) = 0;
};

struct ResourceLoadStorage {
    using Conf = Configuration::Resources;
    Commands::CommandQueue m_Queue;
    StackAllocatorMemory<Conf::QueueMemory> m_Memory;
};

struct BaseAsyncTask {
    struct NotEnoughStorage {
        size_t requiredSpace;//not yet used
    };
    struct RetryLater {};
};

class iAsyncTask : public BaseAsyncTask {
public:
    virtual ~iAsyncTask() {};

    virtual void Do(ResourceLoadStorage &storage) = 0;
};

class FunctionalAsyncTask : public iAsyncTask {
public:
    using TaskFunction = std::function<void(ResourceLoadStorage &)>;

    void Do(ResourceLoadStorage &storage) override final {
        task(storage);
    };


    FunctionalAsyncTask(TaskFunction f) :task(std::move(f)) {}
protected:
    TaskFunction task;
};

template<typename T>
class AsyncSubTask : public FunctionalAsyncTask {
public:
    AsyncSubTask(TaskFunction f, std::shared_ptr<T> owner) : FunctionalAsyncTask(std::move(f)), owner(std::move(owner)) {}
protected:
    std::shared_ptr<T> owner;
};

class iAsyncFileSystemRequest : public std::enable_shared_from_this<iAsyncFileSystemRequest>, public BaseAsyncTask {
public:
    virtual ~iAsyncFileSystemRequest() {};

    virtual void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) = 0;
    virtual void OnTaskQueued(iAsyncLoader *loaderptr) {
        loader = loaderptr;
    }

protected:
    iAsyncLoader *loader;

    void PostTask(FunctionalAsyncTask::TaskFunction func) {
        loader->QueueTask(std::make_shared<AsyncSubTask<iAsyncFileSystemRequest>>(std::move(func), shared_from_this()));
    }
};


class MultiAsyncFileSystemRequest : public iAsyncFileSystemRequest {
public:
    void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override final {
        auto it = handlers.find(requestedURI);
        if (it != handlers.end()) {
            it->second(filedata, storage);
            handlers.erase(it);        
            ++filesProcessed;
            return;
        }
        if (filesProcessed == 0) {
            OnFirstFile(requestedURI, filedata, storage);
            ++filesProcessed;
        }
    }

    using FileHandlerFunctor = std::function<void(StarVFS::ByteTable &, ResourceLoadStorage &)>;
protected:
    void LoadFile(std::string URI, FileHandlerFunctor functor) {
        handlers[URI].swap(functor);
        loader->QueueRequest(std::move(URI), shared_from_this());
    }

    virtual void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) { }
private:
    std::unordered_map<std::string, FileHandlerFunctor> handlers;
    unsigned filesProcessed = 0;
};

} //namespace MoonGlare::Renderer
