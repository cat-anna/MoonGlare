define_static_lib(glfw_context)
target_link_libraries(
  ${TARGET}
  PRIVATE base
          star_vfs
          fmt::fmt
          glfw
          renderer
          engine_core
          glad::glad
          orbit_logger)
