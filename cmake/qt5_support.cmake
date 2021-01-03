get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
execute_process(
  COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
  RESULT_VARIABLE return_code
  OUTPUT_VARIABLE qt5_install_prefix
  OUTPUT_STRIP_TRAILING_WHITESPACE)

set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")
# message(${imported_location})

if(EXISTS ${imported_location})
  add_executable(Qt5::windeployqt IMPORTED)

  set_target_properties(Qt5::windeployqt PROPERTIES IMPORTED_LOCATION
                                                    ${imported_location})
endif()

macro(auto_qt_gen TARGET)
  set_target_properties(${TARGET} PROPERTIES AUTOMOC ON)
  set_target_properties(${TARGET} PROPERTIES AUTOUIC ON)
  set_target_properties(${TARGET} PROPERTIES AUTORCC ON)
endmacro()

function(qt_app TARGET)
  auto_qt_gen(${TARGET})

  set_property(TARGET ${TARGET} PROPERTY WIN32_EXECUTABLE true)

  add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND
      Qt5::windeployqt --no-translations --no-system-d3d-compiler --no-webkit2
      --no-compiler-runtime --no-virtualkeyboard --no-opengl-sw --dir
      "${TARGET_DESTINATTION}"
      "$<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_NAME:${TARGET}>")

  get_target_property(res_path qt-resources SOURCE_DIR)
  file(GLOB_RECURSE icon_res ${res_path}/*.qrc ${res_path}/*.rc)
  target_sources(${TARGET} PRIVATE ${icon_res})
endfunction()
