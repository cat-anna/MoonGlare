file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
set(TARGET mdmp)
add_executable(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC)
target_link_libraries(${TARGET} fmt::fmt assimp::assimp orbit_logger base
                      tool_base)
