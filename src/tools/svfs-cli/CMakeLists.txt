set(TARGET svfs-cli)

file(GLOB_RECURSE SRC *.h *.cpp)
file(GLOB_RECURSE SCRIPTS scripts/*.lua)

add_executable(${TARGET} ${SRC} ${BINARY_RESOURCES})
target_include_directories(${TARGET} PUBLIC include)
target_link_libraries(
  ${TARGET}
  base
  fmt::fmt
  orbit_logger
  sol2::sol2
  star_vfs
  libzippp::libzippp)

embed_resources(${TARGET} ${SCRIPTS})
