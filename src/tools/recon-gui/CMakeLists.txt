if(Qt5_FOUND)

  file(GLOB_RECURSE SRC *.cpp *.h *.hpp)

  set(TARGET recon-gui)

  add_executable(${TARGET} ${SRC})
  target_include_directories(${TARGET} PUBLIC .)
  target_link_libraries(
    ${TARGET}
    PUBLIC fmt::fmt
           pugixml
           Qt5::Core
           Qt5::Widgets
           Qt5::Gui
           orbit_logger
           base
           tool_base
           tool_base_qt)

  qt_app(${TARGET})

endif()
