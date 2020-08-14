function(create_data_package source_directory_name)
  # set(result)

  get_filename_component(source_directory ${source_directory_name} ABSOLUTE)
  set(out_file_name ${TARGET_DESTINATTION}/${source_directory_name}.zip)
  set(package_name ${source_directory_name}.zip)

  message(${out_file_name})

  file(GLOB_RECURSE package_srcs ${source_directory}/*)

  add_custom_target(
    ${package_name}
    COMMAND svfs-cli --action
            "build_package;${source_directory};${out_file_name}"
    WORKING_DIRECTORY ${TARGET_DESTINATTION}
    COMMENT "Building package ${source_directory_name}"
    DEPENDS svfs-cli
    BYPRODUCTS ${out_file_name}
    VERBATIM
    SOURCES ${package_srcs})

endfunction()
