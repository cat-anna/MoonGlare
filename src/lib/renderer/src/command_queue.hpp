#pragma once

#include "memory/stack_allocator.hpp"
#include "renderer/renderer_configuration.hpp"
#include <array>
#include <cassert>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Renderer {

namespace detail {

struct CommandQueueEntry {
    using Functor = void (*)(const void *);

    Functor functor;
    void *memory;

    void Execute() const {
        assert(functor);
        assert(memory);
        functor(memory);
    }

    template <typename T>
    static void CommandExecFunctor(const void *m) {
        reinterpret_cast<const T *>(m)->Execute();
    }
};

} // namespace detail

template <size_t kEntriesLimit_t, typename MemoryAllocator_t>
class alignas(16) BasicCommandQueue final {
public:
    using MemoryAllocator = MemoryAllocator_t;
    static constexpr size_t kEntriesLimit = kEntriesLimit_t;

    using CommandEntry = detail::CommandQueueEntry;

    BasicCommandQueue() = default;
    BasicCommandQueue(const BasicCommandQueue &) = delete;
    BasicCommandQueue &operator=(const BasicCommandQueue &) = delete;

#if 0
    template <typename T> using ByteArray = Memory::StaticMemory<T, Conf::ArgumentMemoryBuffer>;
    using Allocator_t = Memory::StackAllocator<ByteArray>;

    uint32_t CommandsCapacity() const { return Conf::CommandLimit; }
    uint32_t MemoryCapacity() const { return Conf::ArgumentMemoryBuffer; }
    uint32_t CommandsAllocated() const { return allocated_commands; }
    uint32_t MemoryAllocated() const { return static_cast<uint32_t>(m_Memory.Allocated()); }

    bool IsEmpty() const { return CommandsAllocated() == 0; }
    bool IsFull() const { return CommandsAllocated() >= CommandsCapacity(); }

    void SortExecuteClear() {
        Sort();
        Execute();
    }
#endif

    void Execute() {
        for (size_t it = 0; it < allocated_commands; ++it) {
            auto entry = entries[it];
            entry.Execute();
        }
    }

    void MemZero() {
        Clear();
        entries.fill({});
        memory.Zero();
    }

    void ClearAllocation() {
        allocated_commands = commands_preamble;
        memory.PartialClear(memory_preamble);
    }
    void Clear() {
        allocated_commands = 0;
        memory.Clear();
        commands_preamble = 0;
        memory_preamble = 0;
    }
    void SetPreamble() {
        commands_preamble = allocated_commands;
        memory_preamble = static_cast<uint32_t>(memory.Allocated());
    }

#if 0
    void Sort() {
        if (allocated_commands > 1) {
            // m_SortKeys[allocated_commands].m_UIntValue = 0xFFFF;
            SortBegin(static_cast<int>(commands_preamble),
                      static_cast<int>(allocated_commands) - 1);
        }
    }
#endif

    MemoryAllocator &GetMemory() { return memory; }

    template <typename Command_t>
    typename void PushCommand(Command_t &&command) { //CommandKey SortKey = CommandKey()
        MakeCommand<Command_t>(std::forward<Command_t>(command));
    }

    template <typename CMD, typename... ARGS>
    typename CMD *MakeCommand(ARGS &&...args) { //CommandKey SortKey = CommandKey()
        void *argptr = memory.Allocate<CMD>();
        if ( //IsFull() ||
            argptr == nullptr) {
            __debugbreak();
            AddLogf(Warning, "Command queue is full. Command %s has not been allocated!",
                    typeid(CMD).name());
            return nullptr;
        }

        // AddLog(Warning, fmt::format("{}:{}:{}:{}", allocated_commands, sizeof(CMD), argptr,
        //                             typeid(CMD).name()));

        size_t index = allocated_commands;
        ++allocated_commands;

        entries[index] = CommandEntry{
            .functor = &CommandEntry::CommandExecFunctor<CMD>,
            .memory = argptr,
        };

        // m_SortKeys[index] = SortKey;

        auto r = new (argptr) CMD(std::forward<ARGS>(args)...);

        return r;
    }

#if 0
    template <typename CMD, typename... ARGS>
    typename CMD::Argument *MakeCommandKey(CommandKey key, ARGS &&...args) {
        auto *argptr = AllocateMemory<CMD::Argument>(CMD::ArgumentSize());
        if (IsFull() || !argptr) {
            AddLogf(Warning, "Command queue is full. Command %s has not been allocated!",
                    typeid(CMD).name());
            return nullptr;
        }

        size_t index = allocated_commands;
        ++allocated_commands;

        m_CommandFunctions[index] = CMD::GetFunction();
        m_CommandArguments[index] = argptr;
        m_SortKeys[index] = key;

        return new (argptr) typename CMD::Argument{std::forward<ARGS>(args)...};
    }

    template <typename CMD, typename... ARGS>
    typename CMD::Argument *MakeCommand(ARGS &&...args) {
        return MakeCommandKey<CMD>(CommandKey(), std::forward<ARGS>(args)...);
    }

    struct ExecuteQueueArgument {
        CommandQueue *m_Queue;
        static void Execute(const ExecuteQueueArgument *arg) {
            assert(arg->m_Queue);
            arg->m_Queue->Execute(false);
        }
    };
    using ExecuteQueue = CommandTemplate<ExecuteQueueArgument>;

    bool PushQueue(CommandQueue *queue, CommandKey key = CommandKey()) {
        if (!queue) {
            AddLogf(Warning, "Attempt to queue null queue!");
            return false;
        }
        auto *argptr = AllocateMemory<ExecuteQueue::Argument>(
            static_cast<uint32_t>(ExecuteQueue::ArgumentSize()));
        if (IsFull() || !argptr) {
            AddLogf(Warning, "Command queue is full. Command %s has not been allocated!",
                    typeid(ExecuteQueue).name());
            return false;
        }

        size_t index = allocated_commands;
        ++allocated_commands;

        m_CommandFunctions[index] = ExecuteQueue::GetFunction();
        m_CommandArguments[index] = argptr;
        m_SortKeys[index] = key;

        argptr->m_Queue = queue;

        return true;
    }

private:
#endif
    template <typename T>
    using Array = std::array<T, kEntriesLimit>;

    size_t allocated_commands = 0;

    Array<CommandEntry> entries;
    MemoryAllocator memory;
    // Array<CommandKey> sort_keys;

    uint32_t commands_preamble;
    uint32_t memory_preamble;
    // void SortBegin(int first, int last);
};

using DefaultCommandQueueMemoryAllocator =
    Memory::StaticStackAllocator<Configuration::CommandQueue::kArgumentMemoryBuffer>;

using CommandQueue = BasicCommandQueue<Configuration::CommandQueue::kCommandLimit,
                                       DefaultCommandQueueMemoryAllocator>;

struct CommandQueueRef {
    CommandQueueRef(CommandQueue &q) : queue(q) {}
    CommandQueue &queue;
    CommandQueue *operator->() { return &queue; }
    CommandQueue *operator&() const { return &queue; }
};

} // namespace MoonGlare::Renderer
