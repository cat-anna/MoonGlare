#pragma once

#include "Configuration.Renderer.h"
#include "Commands/CommandQueue.h"

namespace MoonGlare::Renderer {

struct ResourceLoadStorage {
    using Conf = Configuration::Resources;
    Commands::CommandQueue m_Queue;
    StackAllocatorMemory<Conf::QueueMemory> m_Memory;
};

class iAsyncFileSystemRequest : public std::enable_shared_from_this<iAsyncFileSystemRequest> {
public:
    virtual ~iAsyncFileSystemRequest() {};

    virtual void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage, iAsyncLoader *loader) = 0;

    //can be thrown: from OnFileReady
    struct NotEnoughStorage {
        size_t requiredSpace;
    };
};

using SharedAsyncFileSystemRequest = std::shared_ptr<iAsyncFileSystemRequest>;

class iAsyncLoader {
public:
    virtual ~iAsyncLoader() {};

    virtual bool AnyJobPending() = 0;
    virtual bool AllResoucecsLoaded() = 0;

    virtual void QueueRequest(std::string URI, SharedAsyncFileSystemRequest handler) = 0;
};

class MultiAsyncFileSystemRequest : public iAsyncFileSystemRequest {
public:
    void OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage, iAsyncLoader *loader) override final {
        auto it = handlers.find(requestedURI);
        if (it != handlers.end()) {
            it->second(filedata, storage, loader);
            handlers.erase(it);        
            ++filesProcessed;
            return;
        }
        if (filesProcessed == 0) {
            OnFirstFile(requestedURI, filedata, storage, loader);
            ++filesProcessed;
        }
    }

    using FileHandlerFunctor = std::function<void(StarVFS::ByteTable &, ResourceLoadStorage &, iAsyncLoader *)>;
protected:
    void LoadFile(iAsyncLoader *loader, std::string URI, FileHandlerFunctor functor) {
        handlers[URI].swap(functor);
        loader->QueueRequest(std::move(URI), shared_from_this());
    }

    virtual void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage, iAsyncLoader *loader) { }
private:
    std::unordered_map<std::string, FileHandlerFunctor> handlers;
    unsigned filesProcessed = 0;
};

} //namespace MoonGlare::Renderer
