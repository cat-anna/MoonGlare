file(GLOB_RECURSE SRC *.cpp *.hpp)
set(TARGET engine)
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
  lua_context)

# add_dependencies(engine all_data_packages)
