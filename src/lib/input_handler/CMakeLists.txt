define_static_lib(input_handler)
target_link_libraries(${TARGET} PRIVATE base fmt::fmt orbit_logger)
