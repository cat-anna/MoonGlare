set(TARGET svfs-cli)

file(GLOB_RECURSE SRC *.h *.cpp)
file(GLOB_RECURSE SCRIPTS scripts/*.lua)

add_executable(${TARGET} ${SRC} ${BINARY_RESOURCES})
target_include_directories(${TARGET} PUBLIC include)
target_link_libraries(
  ${TARGET}
  star_vfs
  base
  tool_base
  orbit_logger
  fmt::fmt
  assimp::assimp
  sol2::sol2
  libzippp::libzippp)

embed_resources(${TARGET} ${SCRIPTS})
