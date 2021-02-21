file(GLOB_RECURSE SRC *.cpp *.hpp)
set(TARGET generic_engine)
add_executable(${TARGET} ${SRC})
target_link_libraries(
  ${TARGET}
  fmt::fmt
  assimp::assimp
  orbit_logger
  base
  star_vfs
  glfw
  glad::glad
  OpenGL::GL
  engine_core
  engine_runner
  lua_context)

add_dependencies(generic_engine build_all_data_packages)
