function(embed_resources target)
  set(result)

  set(path ${CMAKE_CURRENT_BINARY_DIR}/generated/embedded)
  file(MAKE_DIRECTORY ${path})
  target_include_directories(${target}
                             PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

  set(all_res_file ${path}/all_files.hpp)
  list(APPEND result ${all_res_file})

  file(
    WRITE ${all_res_file}
    "// File is generated

#pragma once

#include <stdint.h>

struct EmbeddedFileInfo {
    const unsigned char *data;
    const size_t size;
    const char *file_name;
};

")

  foreach(in_f ${ARGN})
    get_filename_component(src_f ${in_f} ABSOLUTE)
    get_filename_component(out_name ${in_f} NAME)
    string(REPLACE "." "_" out_var_name ${out_name})
    file(APPEND ${all_res_file} "#include \"${out_name}.h\"\n")
  endforeach()

  file(APPEND ${all_res_file} "\n")
  file(APPEND ${all_res_file}
       "static constexpr EmbeddedFileInfo kEmbeddedFiles[] = {\n")

  foreach(in_f ${ARGN})
    get_filename_component(src_f ${in_f} ABSOLUTE)
    get_filename_component(out_name ${in_f} NAME)

    set(out_f "${path}/${out_name}.h")

    string(REPLACE "." "_" out_var_name ${out_name})

    add_custom_command(
      OUTPUT ${out_f}
      COMMAND ${BIN2C_TOOL} -o ${out_f} -n ${out_var_name} ${src_f}
      DEPENDS ${in_f}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Embedding ${out_name}"
      VERBATIM)
    list(APPEND result ${out_f})
    file(APPEND ${all_res_file}
         "\t{ ${out_var_name}, ${out_var_name}_size, \"${out_name}\" },\n")
  endforeach()

  file(APPEND ${all_res_file} "}; \n")

  target_sources(${target} PRIVATE ${result} ${ARGN})
endfunction()
