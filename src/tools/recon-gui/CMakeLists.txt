if(Qt5_FOUND)
  define_executable(
    NAME
    recon-gui
    LINKS
    pugixml
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    orbit_logger
    base
    tool_base
    tool_base_qt)

  target_include_directories(${TARGET} PUBLIC .)
  qt_app(${TARGET})
endif()
