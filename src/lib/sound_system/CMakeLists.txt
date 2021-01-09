set(TARGET sound_system)

file(GLOB_RECURSE SRC src/* include/*)

add_library(${TARGET} STATIC ${SRC})

target_include_directories(${TARGET} PUBLIC include)
target_include_directories(${TARGET} PRIVATE src)

target_link_libraries(${TARGET} PRIVATE base star_vfs fmt::fmt orbit_logger)
target_link_libraries(${TARGET} PUBLIC OpenAL::OpenAL)

if(NOT DEFINED SOUNDSYSTEM_ENABLE_WAVE OR SOUNDSYSTEM_ENABLE_WAVE)
  target_compile_definitions(${TARGET} PRIVATE -DSOUNDSYSTEM_ENABLE_WAVE)
  message("* Soundsystem: decoder WAVE is enabled")
endif()

if(NOT DEFINED SOUNDSYSTEM_ENABLE_LIBMODPLUG OR SOUNDSYSTEM_ENABLE_LIBMODPLUG)
  find_library(MODPLUG_LIB modplug REQUIRED)
  message("* Soundsystem: decoder modplug is enabled (${MODPLUG_LIB})")
  target_link_libraries(${TARGET} PUBLIC ${MODPLUG_LIB})
  target_compile_definitions(${TARGET} PRIVATE -DSOUNDSYSTEM_ENABLE_LIBMODPLUG)
endif()

if(NOT DEFINED SOUNDSYSTEM_ENABLE_LIBMPG123 OR SOUNDSYSTEM_ENABLE_LIBMPG123)
  find_library(MPG123_LIB libmpg123 REQUIRED)
  message("* Soundsystem: decoder mpg123 is enabled (${MPG123_LIB})")
  target_link_libraries(${TARGET} PUBLIC ${MPG123_LIB})
  target_compile_definitions(${TARGET} PRIVATE -DSOUNDSYSTEM_ENABLE_LIBMPG123)
endif()

if(NOT DEFINED SOUNDSYSTEM_ENABLE_LIBFLAC)
  find_package(flac CONFIG)
  set(SOUNDSYSTEM_ENABLE_LIBFLAC ${flac_FOUND})
endif()

if(SOUNDSYSTEM_ENABLE_LIBFLAC)
  message("* Soundsystem: decoder FLAC is enabled")
  target_link_libraries(${TARGET} PUBLIC FLAC::FLAC FLAC::FLAC++)
  target_compile_definitions(${TARGET} PRIVATE -DSOUNDSYSTEM_ENABLE_LIBFLAC)
endif()

if(NOT DEFINED SOUNDSYSTEM_ENABLE_LIBVORBIS)
  find_package(Vorbis CONFIG)
  set(SOUNDSYSTEM_ENABLE_LIBVORBIS ${Vorbis_FOUND})
endif()

if(SOUNDSYSTEM_ENABLE_LIBVORBIS)
  message("* Soundsystem: decoder Vorbis is enabled")
  target_link_libraries(${TARGET} PUBLIC Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile)
  target_compile_definitions(${TARGET} PRIVATE -DSOUNDSYSTEM_ENABLE_LIBVORBIS)
endif()
