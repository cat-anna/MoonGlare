set(TARGET svfs-cli)

file(GLOB_RECURSE SRC *.h *.cpp *.hpp)
file(GLOB_RECURSE SCRIPTS scripts/*.lua)

add_executable(${TARGET} ${SRC} ${SCRIPTS})
target_link_libraries(
  ${TARGET}
  star_vfs
  base
  tool_base
  orbit_logger
  fmt::fmt
  assimp::assimp
  sol2::sol2
  libzippp::libzippp
  sound_system)

embed_resources(${TARGET} ${SCRIPTS})
