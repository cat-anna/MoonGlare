#pragma once

#ifdef DEBUG
#include <intrin.h>
#endif

#define NOMINMAX
#include <locale>
#include <codecvt>

#include <unordered_map>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <array>
#include <bitset>
#include <variant>      
#include <optional>
#include <any>

#include <string>
#include <cstring>
#include <regex>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>

#include <memory>
#include <algorithm>
#include <chrono>
#include <random>
#include <type_traits>
#include <typeindex>

#include <mutex>
#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <pugixml.hpp>

#include <boost/tti/has_member_function.hpp>

#include "Config/Config.h"

#include <lua.hpp>
#include <EngineBase/Script/LuaBridge/LuaBridge.h>

#include <Foundation/OrbitLoggerConf.h>
#include <Foundation/PerfCounters.h>

#include <StarVFS/core/nfStarVFS.h>

#include <libSpace/src/Utils.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
using namespace Space::Utils::HashLiterals;
#include <libSpace/src/Space.h>
#include <libSpace/src/Container/StaticVector.h>
#include <libSpace/src/Memory/Memory.h>
#include <libSpace/src/Memory/StackAllocator.h>
#include <libSpace/src/Memory/BitampAllocator.h>
