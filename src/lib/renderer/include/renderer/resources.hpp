#pragma once

#include "async_loader.hpp"
#include "readonly_file_system.h"
#include "renderer/resources/shader_resource_interface.hpp"
#include "renderer/resources/texture_resource_interface.hpp"
#include "resource_id.hpp"
#include "types.hpp"
#include <gsl/gsl>
#include <memory>
#include <string>

namespace MoonGlare::Renderer {

struct CommandQueueRef;

//TODO: Possibly not needed. Commands shall be buffered into queue feed to rendering thread.
class iContextResourceTask {
public:
    virtual ~iContextResourceTask() = default;
    virtual void Execute(CommandQueueRef &output_queue) = 0;
};

class FunctorContextResourceTask : public iContextResourceTask {
public:
    virtual ~FunctorContextResourceTask() = default;
    FunctorContextResourceTask(std::function<void(CommandQueueRef &output_queue)> f)
        : func(std::move(f)) {}
    void Execute(CommandQueueRef &output_queue) override { func(output_queue); }

private:
    std::function<void(CommandQueueRef &output_queue)> func;
};

class iContextResourceLoader {
public:
    virtual ~iContextResourceLoader() = default;
    virtual void PushResourceTask(std::shared_ptr<iContextResourceTask> task) = 0;

    void PushResourceTask(std::function<void(CommandQueueRef &output_queue)> task) {
        PushResourceTask(std::make_shared<FunctorContextResourceTask>(std::move(task)));
    }
};

class iResourceManager : public iRuntimeResourceLoader,
                         public Resources::iShaderResource,
                         public Resources::iTextureResource {
public:
    virtual ~iResourceManager() = default;

    // virtual void ReloadAll() = 0;

    static std::unique_ptr<iResourceManager>
    CreteDefaultResourceManager(gsl::not_null<iAsyncLoader *> async_loader,
                                gsl::not_null<iContextResourceLoader *> context_loader,
                                gsl::not_null<iReadOnlyFileSystem *> file_system);
};

//TODO
struct MeshResourceHandle {};
struct MeshSource {};

class iMeshManager {
public:
    virtual ~iMeshManager() = default;

    virtual MeshResourceHandle LoadMesh(FileResourceId res_id) = 0;
    virtual MeshResourceHandle CreateMesh(const MeshSource &source, FileResourceId res_id) = 0;
};

} // namespace MoonGlare::Renderer
