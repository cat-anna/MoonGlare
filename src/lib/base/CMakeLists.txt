file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
set(TARGET base)
add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(
  ${TARGET}
  # boost
  fmt::fmt orbit_logger)
