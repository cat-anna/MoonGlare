if(Qt5_FOUND)

  file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
  set(TARGET editor)
  add_executable(${TARGET} ${SRC})
  target_include_directories(${TARGET} PUBLIC)
  target_link_libraries(
    ${TARGET}
    qt-resources
    fmt::fmt
    assimp::assimp
    pugixml
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    orbit_logger
    base
    tool_base
    tool_base_qt
    star_vfs)

  qt_app(${TARGET})

endif()
