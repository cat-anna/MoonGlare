if(Qt5_FOUND)
  file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
  set(TARGET tool_base_qt)
  add_library(${TARGET} ${SRC})
  target_include_directories(${TARGET} PUBLIC .)
  target_link_libraries(
    ${TARGET}
    # boost
    fmt::fmt
    # luajit
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    orbit_logger
    base
    tool_base)

  auto_qt_gen(${TARGET})

endif()
