#pragma once

#include <queue>

#include "Memory/StaticGenerationBuffer.h"
#include "Memory/BitampAllocator.h"

#include "Handle.h"
#include "HandleTable.h"
#include "InterfaceMap.h"

namespace MoonGlare {

template<typename TimerData, uint32_t TimerLimit_v>
class TimerDispatcher {
public:
    static constexpr uint32_t TimerLimit = TimerLimit_v;

    TimerDispatcher(InterfaceMap &ifaceMap) : handleTable(ifaceMap.GetInterface<HandleTable>()) {
        assert(handleTable);
        allocator.ClearAllocation();
        entry.fill({});
    } 
    ~TimerDispatcher() {
        assert(handleTable);
        for (uint32_t index = 0; index < TimerLimit; ++index) 
            if(allocator.IsAllocated(index)) {
                handleTable->Release(entry[index].handle);
            }
        allocator.ClearAllocation();
        entry.fill({});
    }

    Handle SetInterval(float interval, TimerData data) {
        assert(handleTable);
        auto[timer, index] = AllocateTimer();
        if (!timer)
            return {};

        timer->interval = interval;
        timer->privateData = std::move(data);
        timer->expireTime = currentTime + interval;

        queue.emplace(QueueEntry{ timer->expireTime, index });

        return timer->handle;

    }

    Handle SetTimeout(float timeout, TimerData data) {
        assert(handleTable);
        auto[timer, index] = AllocateTimer();
        if (!timer)
            return {};

        timer->interval = -1.0f;
        timer->privateData = std::move(data);
        timer->expireTime = currentTime + timeout;

        queue.emplace(QueueEntry{ timer->expireTime, index });

        return timer->handle;
    }

    void KillTimer(Handle h) {
        assert(handleTable);
        HandleTable::HandlePrivateData hpd;
        if (!handleTable->GetHandleData(h, hpd))
            return;
        uint32_t index = static_cast<uint32_t>(hpd);
        handleTable->Release(h);
        entry[index] = { };
    }

    void SetCurrentTime(float t) { currentTime = t; }

    using ExpireCallbackFunc = void(Handle h, TimerData & timerData);
    template<typename ExpireCallback>
    void Step(float time, ExpireCallback expireCallback) {
        assert(handleTable);
        this->currentTime = time;
        while (!queue.empty()) {
            QueueEntry top = queue.top();
            auto &timer = entry[top.timerIndex];
            if (!timer.valid) {
                queue.pop();
                continue;
            }

            if (timer.expireTime > time)
                break;
            
            queue.pop();
            
            expireCallback(timer.handle, timer.privateData);

            if (timer.interval > 0) {
                timer.expireTime += timer.interval;
                queue.emplace(QueueEntry{ timer.expireTime, top.timerIndex });
            } else {
                KillTimer(timer.handle);
            }
        }
    }

protected:
    template<class T> using Array = std::array<T, TimerLimit>;

    struct TimerEntry {
        bool valid;
        Handle handle;
        float expireTime;
        float interval;
        TimerData privateData;
    };
    struct QueueEntry {
        float expireTime;
        uint32_t timerIndex;

        bool operator<(const QueueEntry& qe) const {
            return expireTime > qe.expireTime;
        }
    };

    HandleTable *handleTable;
    float currentTime = 0.0f;
    std::priority_queue<QueueEntry> queue;
    Memory::LinearAtomicBitmapAllocator<TimerLimit> allocator;
    Array<TimerEntry> entry;

    std::pair<TimerEntry*, uint32_t> AllocateTimer() {
        uint32_t index;
        if (!allocator.Allocate(index))
            return { nullptr, 0 };
        Handle h = handleTable->Allocate(index);

        //TODO:
        //if (!h) {
            //allocator.Release(index);
            //return { nullptr, 0 };
        //}

        auto &timer = entry[index];
        timer = {};
        timer.handle = h;
        timer.valid = true;

        return { &timer, index };
    }
};

}
