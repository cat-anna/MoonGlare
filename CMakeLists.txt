cmake_minimum_required(VERSION 3.10)

project(MoonGlareEngine)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include("add_all_subdirectories")
include("embed_resources")
include("create_data_package")
include("find_external_tools")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT DEFINED TARGET_DESTINATTION)
  set(TARGET_DESTINATTION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif()
message("target destination: " ${TARGET_DESTINATTION})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_DESTINATTION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DGLM_FORCE_RADIANS)

find_package(Boost REQUIRED COMPONENTS filesystem program_options)
find_path(BOOST_INCLUDE_DIR Boost)
include_directories(${BOOST_INCLUDE_DIR})
link_libraries(${Boost_LIBRARIES})

# set(Boost_USE_STATIC_LIBS ON) set(Boost_USE_STATIC_RUNTIME ON)

find_package(fmt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

# find_package(xxhash CONFIG REQUIRED)
find_library(XXHASH_LIBRARY xxhash)
link_libraries(${XXHASH_LIBRARY})

include(FindLua)
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
link_libraries(${LUA_LIBRARIES})

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)

find_package(libzippp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_package(OpenGL REQUIRED)

find_package(openal CONFIG REQUIRED)

# find_package(libmodplug CONFIG REQUIRED)

# set(SOUNDSYSTEM_ENABLE_WAVE FALSE)

# set(SOUNDSYSTEM_ENABLE_LIBMODPLUG FALSE)

# set(SOUNDSYSTEM_ENABLE_LIBMPG123 FALSE)

# set(SOUNDSYSTEM_ENABLE_LIBFLAC FALSE)

# set(SOUNDSYSTEM_ENABLE_LIBVORBIS FALSE)

find_package(
  Qt5
  COMPONENTS Core Widgets Gui Network Charts
  REQUIRED)

if(Qt5_FOUND)
  message("* Qt5 was found")
  include("qt5_support")
else()
  message("* Qt5 is missing")
endif()

if(WIN32)
  add_definitions(-DWINDOWS)
  link_libraries(bcrypt)
elseif(LINUX)
  add_definitions(-DLINUX)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG_DUMP)
  add_definitions(-DDEBUG)
endif()

add_subdirectory(lib)
add_subdirectory(src)
