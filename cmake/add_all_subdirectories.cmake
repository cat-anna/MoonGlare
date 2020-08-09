macro(SUBDIRLIST result curdir)
  file(
    GLOB children
    RELATIVE ${curdir}
    ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(add_all_subdirecties)
  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

  foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
  endforeach()
endmacro()
