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
//std include
#include <locale>
#include <codecvt>

#include <Config/pch_common.h>

#include <boost/optional.hpp>

using std::istream;
using std::ostream;
using std::string;
using wstring = std::wstring;//u16string

#pragma warning ( push, 0 )

#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>    

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#pragma warning ( pop )

#define PUGIXML_NO_XPATH
#define PUGIXML_HAS_LONG_LONG
#define PUGIXML_NO_EXCEPTIONS
#include <pugixml-1.6/src/pugixml.hpp>

#include <libSpace/src/RTTI.h>
#include <libSpace/src/Utils.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
using namespace Space::Utils::HashLiterals;

#include "Serialize.h"

class cRootClass;


#include <luaJiT-2.0.4/lua.hpp>
#if defined(_USE_API_GENERATOR_)
#include <ApiGen/ApiDefAutoGen.h>
#elif defined(_DISABLE_SCRIPT_ENGINE_)
#else
#include <Libs/LuaBridge/LuaBridge.h>
#endif

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
#include <libSpace/src/Container/StaticVector.h>
#include <libSpace/src/Memory/Memory.h>
#include <libSpace/src/Memory/StackAllocator.h>
#include <libSpace/src/Memory/BitampAllocator.h>

#ifdef GLOBAL_CONFIGURATION_FILE
#include GLOBAL_CONFIGURATION_FILE
#endif

#include <Utils/PerfCounters.h>

#pragma warning (error: 4324)

#if 0

#include "Utils/SetGet.h"
#include "Utils/Memory/nMemory.h"
#include "Utils/Memory.h"
#include "Utils/StringUtils.h"
#include "Version.h"
#include "Utils/TemplateUtils.h"
#include "MathConstants.h"
#include "xMath.h"
#include "d2math.h"
#include "Utils/XMLUtils.h"
#include "Utils/StreamUtils.h"

#include "Error.h"

#endif
#endif