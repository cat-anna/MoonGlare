get_filename_component(SCRIPTS_LOCATION ${CMAKE_CURRENT_LIST_DIR}/../scripts ABSOLUTE)

find_program(LUA_EXECUTABLE_LOCATION lua REQUIRED)
execute_process(COMMAND ${LUA_EXECUTABLE_LOCATION} -v OUTPUT_VARIABLE LUA_EXECUTABLE_VERSION_STRING)
string(REGEX MATCH "(Lua [0-9]\\.[0-9]\\.[0-9])" LUA_EXECUTABLE_VERSION "${LUA_EXECUTABLE_VERSION_STRING}")
message("* Using lua interpretter ${LUA_EXECUTABLE_LOCATION} (${LUA_EXECUTABLE_VERSION}) during build")
get_filename_component(LUA_EXECUTABLE_PATH ${LUA_EXECUTABLE} DIRECTORY)

# TODO this is for vcpkg only!
set(LUA_CPATH "${LUA_EXECUTABLE_PATH}/../../bin/?.dll;;")
set(LUA_PATH "${SCRIPTS_LOCATION};;")

set(LUA_INTERPRETTER ${CMAKE_COMMAND} -E env "LUA_CPATH=${LUA_CPATH}" "LUA_PATH=${LUA_PATH}" ${LUA_EXECUTABLE_LOCATION})

function(check_host_lua_module module_name)
  execute_process(
    COMMAND ${LUA_INTERPRETTER} -e "require '${module_name}'"
    OUTPUT_QUIET ERROR_QUIET
    TIMEOUT 0.1
    RESULT_VARIABLE LUA_LFS_FIND_RESULT)

  string(TOUPPER ${module_name} upper_module_name)
  set(var_name "LUA_HAS_${upper_module_name}")

  if(LUA_LFS_FIND_RESULT EQUAL 1)
    message("* Host lua module missing: ${module_name}")
    set(${var_name}
        FALSE
        PARENT_SCOPE)
  else()
    message("* Host lua module found: ${module_name}")
    set(${var_name}
        TRUE
        PARENT_SCOPE)
  endif()
endfunction()

check_host_lua_module(lfs)

set(BIN2C_TOOL ${LUA_INTERPRETTER} ${SCRIPTS_LOCATION}/bin2c.lua)
set(DOC_GEN_TOOL ${LUA_INTERPRETTER} ${SCRIPTS_LOCATION}/doc_gen.lua)
