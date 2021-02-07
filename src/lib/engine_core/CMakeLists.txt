define_static_lib_with_ut(engine_core)
target_link_libraries(
  ${TARGET}
  PUBLIC base
         star_vfs
         fmt::fmt
         orbit_logger
         renderer
         glfw_context
         input_handler
         lua_context)
