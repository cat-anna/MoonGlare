/*
 * Release.h
 *
 *  Created on: 06-12-2013
 *      Author: Paweu
 */

#ifndef RELEASE_H_
#define RELEASE_H_

#ifndef RELEASE
#error Attempt to include release header on non release build
#endif

#define BreakPoint					VoidAction

#define DEBUG_MSG(A, MSG)
#define MINOR_ASSERT(COND, A, ACTION)

#define THROW_ASSERT(CHECK, MSG)	VoidAction
#define THROW_ASSERTs(...)			VoidAction

#define GABI_SINGLETON_CHECK		VoidAction
#define GABI_SINGLETON_SET_CHECK	VoidAction

#define REQUIRE_REIMPLEMENT			VoidAction
//#define REQUIRE_REIMPLEMENT		//should not be defined. Forbidden in release code.

namespace Config {
namespace Release {

void Initialize();
void Finalize();

struct EnableFlags {
	enum {
		Physics = 1,							//<! Release global config: Allways enabled
		PhysicsDebugDraw = 0,					//<! Release global config: Allways disabled

		ShowTitleBarDebugInfo = 0,				//<! Release global config: Allways disabled
	};
};

struct DebugMemoryInterface {
	template<class ... ARGS> void DebugMemorySetClassName(ARGS ... args) {}
	template<class ... ARGS> void DebugMemoryRegisterCounter(ARGS ... args) {}
	DebugMemoryInterface* GetNext() { return nullptr; }
	DebugMemoryInterface* GetPrev() { return nullptr; }
};

} //namespace Release
} //namespace Config


#endif // RELEASE_H_
