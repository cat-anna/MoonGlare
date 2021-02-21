define_static_lib(engine_runner)
target_link_libraries(
  ${TARGET}
  PUBLIC base
         star_vfs
         fmt::fmt
         orbit_logger
         renderer
         engine_core
         glfw_context
         lua_context)
