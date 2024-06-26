#pragma once

#include "Commands/CommandQueue.h"
#include "Configuration.Renderer.h"
#include <functional>
#include <iFileSystem.h>
#include <string>

namespace MoonGlare::Renderer {

struct ResourceLoadStorage {
    using Conf = Configuration::Resources;
    Commands::CommandQueue m_Queue;
    StackAllocatorMemory<Conf::QueueMemory> m_Memory;
};

class iAsyncFileSystemRequest;
using SharedAsyncFileSystemRequest = std::shared_ptr<iAsyncFileSystemRequest>;

class iAsyncTask;
using SharedAsyncTask = std::shared_ptr<iAsyncTask>;

//---------------------------------------------------------------------------------------

class iAsyncLoader {
public:
    virtual ~iAsyncLoader(){};

    virtual void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) = 0;
    virtual void QueueTask(SharedAsyncTask task) = 0;

    using FileRequestFunc =
        std::function<void(const std::string &uri, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage)>;
    void QueueRequest(std::string URI, FileRequestFunc request);

    using TaskFunc = std::function<void(ResourceLoadStorage &storage)>;
    void PostTask(TaskFunc func);
};

//---------------------------------------------------------------------------------------

struct BaseAsyncTask {
    struct NotEnoughStorage {
        size_t requiredSpace; // not yet used
    };
    struct RetryLater {};
};

class iAsyncTask : public BaseAsyncTask {
public:
    virtual ~iAsyncTask(){};

    virtual void Do(ResourceLoadStorage &storage) = 0;
};

class FunctionalAsyncTask : public iAsyncTask {
public:
    using TaskFunction = iAsyncLoader::TaskFunc;

    void Do(ResourceLoadStorage &storage) override final { task(storage); };

    FunctionalAsyncTask(TaskFunction f) : task(std::move(f)) {}

protected:
    TaskFunction task;
};

template <typename T> class AsyncSubTask : public FunctionalAsyncTask {
public:
    AsyncSubTask(TaskFunction f, std::shared_ptr<T> owner)
        : FunctionalAsyncTask(std::move(f)), owner(std::move(owner)) {}

protected:
    std::shared_ptr<T> owner;
};

//---------------------------------------------------------------------------------------

class iAsyncFileSystemRequest : public std::enable_shared_from_this<iAsyncFileSystemRequest>, public BaseAsyncTask {
public:
    virtual ~iAsyncFileSystemRequest(){};

    virtual void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata,
                             ResourceLoadStorage &storage) = 0;
    virtual void OnTaskQueued(iAsyncLoader *loaderptr) { loader = loaderptr; }

protected:
    iAsyncLoader *loader;

    void PostTask(FunctionalAsyncTask::TaskFunction func) {
        loader->QueueTask(std::make_shared<AsyncSubTask<iAsyncFileSystemRequest>>(std::move(func), shared_from_this()));
    }
};

class FunctionalAsyncFileSystemRequest : public iAsyncFileSystemRequest {
    iAsyncLoader::FileRequestFunc func;

public:
    FunctionalAsyncFileSystemRequest(iAsyncLoader::FileRequestFunc f) : func(std::move(f)) { assert(func); }

    virtual void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata,
                             ResourceLoadStorage &storage) {
        func(requestedURI, filedata, storage);
    }
};

class MultiAsyncFileSystemRequest : public iAsyncFileSystemRequest {
public:
    void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata,
                     ResourceLoadStorage &storage) override final {
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

    virtual void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata,
                             ResourceLoadStorage &storage) {}

private:
    std::unordered_map<std::string, FileHandlerFunctor> handlers;
    unsigned filesProcessed = 0;
};

//---------------------------------------------------------------------------------------

inline void iAsyncLoader::QueueRequest(std::string URI, iAsyncLoader::FileRequestFunc request) {
    QueueRequest(std::move(URI), std::make_shared<FunctionalAsyncFileSystemRequest>(std::move(request)));
}

inline void iAsyncLoader::PostTask(iAsyncLoader::TaskFunc func) {
    QueueTask(std::make_shared<FunctionalAsyncTask>(std::move(func)));
}

} // namespace MoonGlare::Renderer
