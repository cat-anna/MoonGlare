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
using std::istream;
using std::ostream;
using std::string;
using wstring = std::u16string;
using LockGuard = std::lock_guard < std::mutex > ;

#define MERGE_(a,b)  a##b
#define LABEL_(a) MERGE_(unique_name_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

#define LOCK_MUTEX_MERGE(a, b) a ## b
#define LOCK_MUTEX_LABEL(name) LOCK_MUTEX_MERGE(__lock_, name)
#define LOCK_MUTEX_LABEL_UNIQUE LOCK_MUTEX_LABEL(__LINE__)

#define LOCK_MUTEX_NAMED(mutex, name) std::lock_guard < decltype(mutex) > name (mutex)
#define LOCK_MUTEX(mutex) LOCK_MUTEX_NAMED(mutex, LOCK_MUTEX_LABEL_UNIQUE)

#define thread_local __declspec(thread)

#undef min
#undef max

#pragma warning ( push, 0 )

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <bullet3-master/src/btBulletDynamicsCommon.h>
#include <bullet3-master/src/btBulletCollisionCommon.h>

//#include <bullet-2.82/src/btBulletDynamicsCommon.h>
//#include <bullet-2.82/src/btBulletCollisionCommon.h>
#pragma warning ( pop )

#define GLM_FORCE_RADIANS
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#define PUGIXML_NO_XPATH
#define PUGIXML_HAS_LONG_LONG
#define PUGIXML_NO_EXCEPTIONS
#include <pugixml-1.6/src/pugixml.hpp>
using pugi::xml_node;
using pugi::xml_attribute;
using pugi::xml_document;

#include <GabiLib/src/GabiLib.h>
#include <GabiLib/src/math/D2Math.h>
#include <GabiLib/src/rtti/RTTI.h>
#include <GabiLib/src/rtti/TemplateClassList.h>
#include <GabiLib/src/rtti/DynamicClassRegister.h>
#include <GabiLib/src/rtti/Exceptions.h>
#include <GabiLib/src/utils/SharedString.h>
#include <GabiLib/src/utils/EnumConverter.h>
#include <GabiLib/src/utils/Serialize.h>

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
#include "Thread.h"
#include "Log.h"
#include "Error.h"

#define DISABLE_COPY() public: ThisClass(const ThisClass&) = delete; ThisClass& operator=(const ThisClass&) = delete

#define AS_STRING(X) #X

#ifdef DEBUG
#define ERROR_STR				"{badstr in " __FUNCTION__ " at " AS_STRING(__LINE__) "}"
#else
#define ERROR_STR				"{?}"
#endif

#endif
