file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
set(TARGET tool_base)
add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(
  ${TARGET}
  # boost
  fmt::fmt
  # luajit
  orbit_logger
  base
  resources
  star_vfs)
