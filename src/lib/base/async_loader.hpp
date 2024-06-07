#pragma once

#include "resource_id.hpp"
#include <functional>
#include <memory>
#include <string>

namespace MoonGlare {

class iAsyncFileSystemRequest;
using SharedAsyncFileSystemRequest = std::shared_ptr<iAsyncFileSystemRequest>;

using FileLoadFunctor = std::function<void(FileResourceId, std::string &file_data)>;

//---------------------------------------------------------------------------------------

class iAsyncTask {
public:
    virtual ~iAsyncTask() = default;
    virtual void DoWork() = 0;
};

class iAsyncTask;
using SharedAsyncTask = std::shared_ptr<iAsyncTask>;

//---------------------------------------------------------------------------------------

class iAsyncLoader;

class iAsyncLoaderObserver : public std::enable_shared_from_this<iAsyncLoaderObserver> {
public:
    virtual ~iAsyncLoaderObserver(){};

    virtual void OnFinished(iAsyncLoader *loader){};
    virtual void OnStarted(iAsyncLoader *loader){};
};

using SharedAsyncLoaderObserver = std::shared_ptr<iAsyncLoaderObserver>;
using WeakAsyncLoaderObserver = std::weak_ptr<iAsyncLoaderObserver>;

//---------------------------------------------------------------------------------------

class iAsyncLoader {
public:
    virtual ~iAsyncLoader() = default;

    using AsyncTaskFunctor = std::function<void()>;

    virtual void SetObserver(SharedAsyncLoaderObserver) = 0;

    // virtual void LoadFile(FileResourceId res_id, SharedAsyncFileSystemRequest handler) = 0;
    virtual void LoadFile(FileResourceId res_id, FileLoadFunctor functional_handler) = 0;

    // virtual void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) = 0;
    virtual void QueueTask(SharedAsyncTask task) = 0;
    virtual void QueueTask(AsyncTaskFunctor task) = 0;

    [[deprecated]] virtual void QueueRequest(std::string URI,
                                             SharedAsyncFileSystemRequest handler) = 0;

    struct AsyncLoaderStatus {
        size_t pending_jobs = 0;
        size_t local_job_count = 0;
        size_t processed_jobs = 0;
    };
    virtual AsyncLoaderStatus GetStatus() const = 0;
};

#ifdef WANTS_GTEST_MOCKS
struct AsyncLoaderMock : public iAsyncLoader {
    MOCK_METHOD1(SetObserver, void(SharedAsyncLoaderObserver));
    MOCK_METHOD2(LoadFile, void(FileResourceId res_id, FileLoadFunctor functional_handler));
    MOCK_METHOD1(QueueTask, void(SharedAsyncTask task));
    MOCK_METHOD1(QueueTask, void(AsyncTaskFunctor task));
    MOCK_METHOD2(QueueRequest, void(std::string URI, SharedAsyncFileSystemRequest handler));
    MOCK_CONST_METHOD0(GetStatus, AsyncLoaderStatus());
};
#endif

//---------------------------------------------------------------------------------------

class FunctionalAsyncTask : public iAsyncTask {
public:
    using AsyncTaskFunctor = iAsyncLoader::AsyncTaskFunctor;
    void DoWork() override final { task(); };
    FunctionalAsyncTask(AsyncTaskFunctor f) : task(std::move(f)) {}

protected:
    AsyncTaskFunctor task;
};

template <typename T>
class AsyncSubTask : public FunctionalAsyncTask {
public:
    AsyncSubTask(AsyncTaskFunctor f, std::shared_ptr<T> owner)
        : FunctionalAsyncTask(std::move(f)), owner(std::move(owner)) {}

protected:
    std::shared_ptr<T> owner;
};

//---------------------------------------------------------------------------------------

class iAsyncFileSystemRequest : public std::enable_shared_from_this<iAsyncFileSystemRequest> {
public:
    virtual ~iAsyncFileSystemRequest(){};

    virtual void OnFileReady(const std::string &requestedURI, std::string &filedata) = 0;
    virtual void OnTaskQueued(iAsyncLoader *loaderptr) { loader = loaderptr; }

protected:
    iAsyncLoader *loader;

    void PostTask(std::function<void()> func) {
        loader->QueueTask(std::make_shared<AsyncSubTask<iAsyncFileSystemRequest>>(
            std::move(func), shared_from_this()));
    }
};

class MultiAsyncFileSystemRequest : public iAsyncFileSystemRequest {
public:
    void OnFileReady(const std::string &requestedURI, std::string &filedata) override final {
        auto it = handlers.find(requestedURI);
        if (it != handlers.end()) {
            it->second(requestedURI, filedata);
            handlers.erase(it);
            ++filesProcessed;
            return;
        }
        if (filesProcessed == 0) {
            OnFirstFile(requestedURI, filedata);
            ++filesProcessed;
        }
    }

    using FileHandlerFunctor = std::function<void(std::string URI, std::string &)>;

protected:
    void LoadFile(std::string URI, FileHandlerFunctor functor) {
        handlers[URI].swap(functor);
        loader->QueueRequest(std::move(URI), shared_from_this());
    }

    virtual void OnFirstFile(const std::string &requestedURI, std::string &filedata) {}

private:
    std::unordered_map<std::string, FileHandlerFunctor> handlers;
    unsigned filesProcessed = 0;
};

} // namespace MoonGlare
