file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
set(TARGET renderer)
add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(
  ${TARGET}
  base
  star_vfs
  fmt::fmt
  glfw
  glad::glad
  orbit_logger)
