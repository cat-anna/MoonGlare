file(GLOB_RECURSE SRC *.cpp *.hpp)

set(TARGET engine_core)

add_library(${TARGET} ${SRC})

target_include_directories(${TARGET} PUBLIC .)

target_link_libraries(${TARGET} base star_vfs fmt::fmt orbit_logger renderer)
