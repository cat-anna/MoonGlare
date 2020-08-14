set(TARGET star_vfs)

file(
  GLOB_RECURSE
  SRC
  include/*.h
  include/*.hpp
  src/*.cpp
  src/*.hpp
  src/*.h)

add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC include)

target_link_libraries(${TARGET} base orbit_logger fmt::fmt libzippp::libzippp)
