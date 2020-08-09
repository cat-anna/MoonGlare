file(GLOB_RECURSE SRC *.cpp *.h *.hpp)
set(TARGET mdmp)
add_executable(${TARGET} ${SRC})
target_include_directories(${TARGET} PUBLIC)
target_link_libraries(
  ${TARGET} fmt::fmt assimp::assimp
  # Boost
  orgit_logger base tool_base)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_DESTINATTION}) install(TARGETS
# ${TARGET} DESTINATION ${TARGET_DESTINATTION})
