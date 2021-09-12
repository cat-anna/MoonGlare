file(GLOB_RECURSE SRC *.cpp *.hpp)
set(TARGET renderer)
add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC include)
target_include_directories(${TARGET} PRIVATE SRC)
target_link_libraries(
  ${TARGET}
  base
  star_vfs
  fmt::fmt
  glfw
  # glad::glad
  engine_core
  orbit_logger)
