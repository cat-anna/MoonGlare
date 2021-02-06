if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options("/std:c++latest")
  message("* MSVC: version ${MSVC_VERSION}")
  message("* MSVC: Enabling c++latest")
else()
  message(FATAL "Add support for current compiler!")
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

# set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)
