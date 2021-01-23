add_custom_target(all_gen_doc_targets)

function(create_gen_doc_target target_name)
  if(NOT LUA_HAS_LFS)
    message("* LFS lua module required by gen doc target ${target_name}")
    return()
  endif()

  set(output_path ${TARGET_DESTINATTION}/gen_doc/${target_name})
  file(MAKE_DIRECTORY ${output_path})

  get_filename_component(source_directory ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)

  add_custom_target(
    ${target_name}
    COMMAND ${DOC_GEN_TOOL} --input=${source_directory} --output=${output_path}
    WORKING_DIRECTORY ${output_path}
    COMMENT "Processing gen doc ${target_name}"
    VERBATIM)

  add_dependencies(all_gen_doc_targets ${target_name})
endfunction()
