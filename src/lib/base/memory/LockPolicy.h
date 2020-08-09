#pragma once

#include <mutex>

namespace MoonGlare::Memory {

struct MemoryNoLockPolicy {
	struct Guard_t { Guard_t(MemoryNoLockPolicy&) {} };
	void lock() {}
	void unlock() {}
};

template<class MUTEX>
struct MemoryUniqueScopedLockPolicy : public MUTEX {
	using Mutex_t = MUTEX;
	using Guard_t = std::lock_guard < MUTEX >;
private:
};

using MutexLockPolicy = MemoryUniqueScopedLockPolicy<std::mutex>;
using NoLockPolicy = MemoryNoLockPolicy;

using DefaultMultiThreadedLockPolicy = MutexLockPolicy;
using DefaultSingleThreadedLockPolicy = MemoryNoLockPolicy;
using DefaultLockPolicy = DefaultSingleThreadedLockPolicy;

} //namespace Memory
