#pragma once

#include "Device/Types.h"
#include "Handles.h"

#include "Exceptions.h"

#include <Memory/StackAllocator.h>

namespace MoonGlare::Renderer {

class iContext;
class iContextInputHandler;
class iRendererFacade;

using StackAllocator = Memory::StackAllocator<Memory::StaticTableMemory>;

// template<uint32_t SIZE>
// struct StackAllocatorMemory {
//     StackAllocator m_Allocator;
//     StackAllocator::Item_t m_Memory[SIZE];

//     StackAllocatorMemory() : m_Allocator(m_Memory, SIZE)  {
//         m_Allocator.Clear();
//         memset(m_Memory, 0, sizeof(m_Memory));
//     }
// };

// namespace Commands {

// struct CommitCommandQueue {
//     CommandQueue *m_Queue;
//     std::atomic<bool> m_Commited;
// };
// }

} //namespace MoonGlare::Renderer
