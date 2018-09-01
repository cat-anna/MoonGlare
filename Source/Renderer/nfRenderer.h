#pragma once

#include "Device/Types.h"
#include "Handles.h"

#include "Exceptions.h"

namespace MoonGlare::Renderer {

class iContext;
class iContextInputHandler;
class iRendererFacade;

using StackAllocator = ::Space::Memory::StackAllocator<::Space::Memory::StaticTableMemory>;

template<uint32_t SIZE>
struct StackAllocatorMemory {
    StackAllocator m_Allocator;
    StackAllocator::Item_t m_Memory[SIZE];

    StackAllocatorMemory() : m_Allocator(m_Memory, SIZE)  {
        m_Allocator.Clear();
        memset(m_Memory, 0, sizeof(m_Memory));
    }
};

class RendererFacade;
class Frame;
class RenderDevice;
class ScriptApi;

class iAsyncLoader;
class iAsyncFileSystemRequest;

class alignas(16) TextureRenderTask;

struct alignas(16) VirtualCamera;

namespace Commands {
    class alignas(16) CommandQueue;

    struct CommitCommandQueue {
        CommandQueue *m_Queue;
        std::atomic<bool> m_Commited;
    };
}

namespace Resources {
    class ResourceManager;
    class TextureResource;
    class MaterialManager;
    class MeshManager;
}

} //namespace MoonGlare::Renderer
