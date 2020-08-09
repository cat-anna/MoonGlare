set(TARGET filesystem)

file(
  GLOB_RECURSE
  SRC
  include/*.h
  include/*.hpp
  src/core/*.cpp
  src/core/*.hpp
  src/core/*.h)

add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC include)

target_link_libraries(${TARGET})

add_subdirectory(tools)
