if(Qt5_FOUND)

  file(GLOB_RECURSE icon_res *.cpp *.h *.hpp *.qrc)

  set(TARGET qt-resources)
  add_library(${TARGET} STATIC ${SRC})
  target_include_directories(${TARGET} PUBLIC .)
  target_link_libraries(${TARGET} Qt5::Core Qt5::Widgets Qt5::Gui)
  auto_qt_gen(${TARGET})

endif()
