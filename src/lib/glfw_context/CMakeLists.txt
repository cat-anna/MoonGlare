define_static_lib(glfw_context)
target_link_libraries(
  ${TARGET}
  PRIVATE base
          star_vfs
          fmt::fmt
          glfw
          renderer
          input_handler
          glad::glad
          orbit_logger)
