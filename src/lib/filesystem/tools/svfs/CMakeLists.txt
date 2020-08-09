set(TARGET svfs)

file(
  GLOB_RECURSE
  SRC
  *.h
  *.cpp
  )

add_executable()(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC include)

target_link_libraries(${TARGET})
