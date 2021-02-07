#pragma once

#include <atomic>

#include "../Configuration.h"

namespace Space {
namespace Memory {

struct LockLessLockPolicy {
	struct Guard {};

	Guard Lock() const { return Guard(); }
};

struct NoVariableProtectionPolicy {
	template<class T>
	using VariableProtector = T;
};

struct NoLockPolicy : public LockLessLockPolicy, public NoVariableProtectionPolicy {
};

using DefaultLockPolicy = NoLockPolicy;

//-----------------------------------------------------------------------------

struct AtomicSpinlockPolicy {
	//struct Guard {
	//};
	//TODO
	//struct LockType {
	//	Guard Lock() const { return Guard(); }
	//};
};

struct AtomicVariableProtectionPolicy {
	template<class T>
	using VariableProtector = std::atomic<T>;
};

struct AtomicLockPolicy : public AtomicSpinlockPolicy, public NoVariableProtectionPolicy {

};

}
}
