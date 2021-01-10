file(GLOB_RECURSE SRC *.cpp *.hpp)
file(GLOB_RECURSE SCRIPTS *.lua)

set(TARGET lua_context)
add_library(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC include)
target_include_directories(${TARGET} PRIVATE src)
target_link_libraries(${TARGET} base star_vfs fmt::fmt orbit_logger)

embed_resources(${TARGET} ${SCRIPTS})
