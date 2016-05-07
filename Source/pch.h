#ifndef PCH_H
#define PCH_H

#pragma warning ( disable: 4005 )
#pragma warning ( disable: 4800 )
#pragma warning ( disable: 4100 )
#pragma warning ( disable: 4505 )
#pragma warning ( disable: 4996 )
#pragma warning ( disable: 4702 )
#pragma warning ( disable: 4127 )
//warning C4201: nonstandard extension used : nameless struct/union
#pragma warning ( disable: 4201 )

#ifdef DEBUG
#include <intrin.h>
#endif
//gl include
//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glfx.h>
//std include
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <list>
#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <locale>
#include <codecvt>
#include <atomic>
#include <random>
#include <type_traits>
using std::istream;
using std::ostream;
using std::string;
using wstring = std::wstring;//u16string
using LockGuard = std::lock_guard < std::mutex > ;

#define MERGE_(a,b)  a##b
#define LABEL_(a) MERGE_(unique_name_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

#define LOCK_MUTEX_MERGE(a, b) a ## b
#define LOCK_MUTEX_LABEL(name) LOCK_MUTEX_MERGE(__lock_, name)
#define LOCK_MUTEX_LABEL_UNIQUE LOCK_MUTEX_LABEL(__LINE__)

#define LOCK_MUTEX_NAMED(mutex, name) std::lock_guard < decltype(mutex) > name (mutex)
#define LOCK_MUTEX(mutex) LOCK_MUTEX_NAMED(mutex, LOCK_MUTEX_LABEL_UNIQUE)

//#define thread_local __declspec(thread)

#undef min
#undef max

#pragma warning ( push, 0 )

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/locale.hpp>

#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>    

#include <bullet3-master/src/btBulletDynamicsCommon.h>
#include <bullet3-master/src/btBulletCollisionCommon.h>
//#include <bullet-2.82/src/btBulletDynamicsCommon.h>
//#include <bullet-2.82/src/btBulletCollisionCommon.h>

#define GLM_FORCE_RADIANS
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#pragma warning ( pop )

#define PUGIXML_NO_XPATH
#define PUGIXML_HAS_LONG_LONG
#define PUGIXML_NO_EXCEPTIONS
#include <pugixml-1.6/src/pugixml.hpp>
using pugi::xml_node;
using pugi::xml_attribute;
using pugi::xml_document;

#include <libSpace/src/RTTI.h>
#include <libSpace/src/Utils.h>

#include "Serialize.h"

//scripts
#include <luaJiT/lua.hpp>
#if defined(_USE_API_GENERATOR_)
#	include <ApiGen/ApiDefAutoGen.h>
#elif defined(_DISABLE_SCRIPT_ENGINE_)
#else
#	include <LuaBridge/LuaBridge.h>
#endif

using StringVector = std::vector < string > ;
using StringDeque = std::deque < string > ;
using StringList = std::list < string > ;
using StringStringMap = std::unordered_map < string, string > ;

class cRootClass;

#include "Utils/SetGet.h"
#include "Utils/Memory/nMemory.h"
#include "Utils/Memory.h"
#include "Utils/ActionQueue.h"
#include "Utils/StringUtils.h"
#include "Version.h"
#include "TemplateUtils.h"
#include "MathConstants.h"
#include "xMath.h"
#include "d2math.h"
#include "XMLUtils.h"
#include "StreamUtils.h"
#include "Utils/LuaUtils.h"

namespace Core {
	class Console;
#if defined(_USE_API_GENERATOR_)
	typedef ApiDefAutoGen::Namespace ApiInitializer;
#elif defined(_DISABLE_SCRIPT_ENGINE_)
	struct DummyApiInitializer {
		template <class ... Args1, class ... Args2> DummyApiInitializer& beginClass(Args2 ... args) { return *this; }
		template <class ... Args1, class ... Args2> DummyApiInitializer& deriveClass(Args2 ... args) { return *this; }
		template <class ... Args1, class ... Args2> DummyApiInitializer& endClass(Args2 ... args) { return *this; }
		template <class ... Args1, class ... Args2> DummyApiInitializer& beginNamespace(Args2 ... args) { return *this; }
		template <class ... Args1, class ... Args2> DummyApiInitializer& endNamespace(Args2 ... args) { return *this; }

		template <class ... Args1, class ... Args2> DummyApiInitializer& addFunction(Args2 ... args) { return *this; }
		template <class ... Args1, class ... Args2> DummyApiInitializer& addVariable(Args2 ... args) { return *this; }
	};
	typedef DummyApiInitializer ApiInitializer;
#else
	typedef luabridge::Namespace ApiInitializer;
#endif
}

using Core::ApiInitializer;

#include "Config/Config.h"
#include <StarVFS/core/nfStarVFS.h>
#include <OrbitLogger/src/OrbitLogger.h>

#include <libSpace/src/Space.h>

#ifdef GLOBAL_CONFIGURATION_FILE
#include GLOBAL_CONFIGURATION_FILE
#endif

#include "Error.h"

#define CriticalCheck(COND, MSG)					do { if(!(COND)) { AddLogf(Error, "Critical check failed!!! condition '%s' returned false. Error message: '%s'", #COND, (MSG?MSG:"No error message")); throw MSG; } } while(0)

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_

#define __LOG_ACTION_F_Performance(T, ...)			ORBITLOGGER_BeginLogf(Hint, 0, __VA_ARGS__)
#define __LOG_ACTION_Performance(T, A)				ORBITLOGGER_BeginLog(Hint, 0, A)

namespace PerformanceCounters {
inline void PrintPerfCounter(unsigned __int64 Value, void *OwnerPtr, const char *OwnerName, const char *Name) {
	//AddLogf(Performance, "Destroying counter %s. Owner %s (%x). Current value: %llu", Name, OwnerName, OwnerPtr, static_cast<unsigned long long>(Value));
}

template<class OWNER, class INFO>
struct Counter {
	OWNER *Owner = nullptr;
	unsigned __int64 Value = 0;
	void increment(unsigned __int64 val = 1) { Value += val; }
	void decrement(unsigned __int64 val = 1) { Value -= val; }
	void SetOwner(OWNER *ptr) {
		Owner = ptr;
	}
	~Counter() {
		const char *OwnerName;
		if (Owner)
			OwnerName = Owner->GetDynamicTypeInfo()->GetName();
		else
			OwnerName = OWNER::GetStaticTypeInfo()->GetName();
		PrintPerfCounter(Value, Owner, OwnerName, INFO::Name());
	}
};
}

#define DeclarePerformanceCounter(NAME)			\
private:										\
	struct __PerfCounterInfo_##NAME	{			\
		static const char* Name() { return #NAME; }\
	};											\
	PerformanceCounters::Counter<ThisClass, __PerfCounterInfo_##NAME> m_PerfCounter##NAME

#define IncrementPerformanceCounter(NAME)		do { m_PerfCounter##NAME.increment(); } while(0)
#define DecrementPerformanceCounter(NAME)		do { m_PerfCounter##NAME.decrement(); } while(0)
#define PerformanceCounter_inc(NAME, VALUE)		do { m_PerfCounter##NAME.increment(VALUE); } while(0)
#define PerformanceCounter_dec(NAME, VALUE)		do { m_PerfCounter##NAME.decrement(VALUE); } while(0)
#define SetPerformanceCounterOwner(NAME)		do { m_PerfCounter##NAME.SetOwner(this); } while(0)
#else
#define DeclarePerformanceCounter(NAME)	
#define IncrementPerformanceCounter(NAME)		do { /* nothing there */ } while(0)
#define DecrementPerformanceCounter(NAME)		do { /* nothing there */ } while(0)
#define SetPerformanceCounterOwner(NAME)		do { /* nothing there */ } while(0)
#define __LOG_ACTION_F_Performance(T, ...)		ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_Performance(T, A)			ORBITLOGGER_DISASBLED_ACTION()
#endif


#define DISABLE_COPY() public: ThisClass(const ThisClass&) = delete; ThisClass& operator=(const ThisClass&) = delete

#define AS_STRING(X) #X

#ifdef DEBUG
#define ERROR_STR				"{badstr in " __FUNCTION__ " at " AS_STRING(__LINE__) "}"
#else
#define ERROR_STR				"{?}"
#endif

#endif
