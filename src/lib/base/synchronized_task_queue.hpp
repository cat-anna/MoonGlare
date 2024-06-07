#pragma once

#include <list>
#include <mutex>

namespace MoonGlare {

template <typename Task>
struct SynchronizedTaskQueue {
public:
    using ContainerType = std::list<Task>;

    void Push(Task task) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.emplace_back(std::move(task));
    }

    Task Pop() {
        std::lock_guard<std::mutex> lock(mutex);
        auto item = std::move(queue.front());
        queue.pop_front();
        return item;
    }

    bool Pop(Task &out) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }
        out = std::move(queue.front());
        queue.pop_front();
        return true;
    }

    size_t QueuedCount() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

    bool Empty() const { return queue.empty(); }

    void Swap(ContainerType &out) {
        std::lock_guard<std::mutex> lock(mutex);
        out.swap(queue);
    }

private:
    ContainerType queue;
    mutable std::mutex mutex;
};

} // namespace MoonGlare
