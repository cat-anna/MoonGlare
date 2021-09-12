define_executable(
  NAME
  generic_engine
  LINKS
  assimp::assimp
  orbit_logger
  base
  star_vfs
  glfw
  glad::glad
  OpenGL::GL
  freeimage::FreeImage
  engine_core
  engine_runner
  lua_context)

add_dependencies(generic_engine build_all_data_packages)
