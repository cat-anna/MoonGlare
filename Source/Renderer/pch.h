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

#include <Config/pch_common.h>

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
#include <pugixml.hpp>

#include <libSpace/src/RTTI.h>
#include <libSpace/src/Utils.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
using namespace Space::Utils::HashLiterals;

class cRootClass;

#include "Config/Config.h"
#include <OrbitLogger/src/OrbitLogger.h>
#include <StarVFS/core/nfStarVFS.h>

#include <libSpace/src/Space.h>

#include <libSpace/src/Memory/Memory.h>
#include <libSpace/src/Memory/StackAllocator.h>
#include <libSpace/src/Memory/BitampAllocator.h>

#ifdef GLOBAL_CONFIGURATION_FILE
#include GLOBAL_CONFIGURATION_FILE
#endif

#include <Foundation/PerfCounters.h>

#pragma warning (error: 4324)

#endif