add_custom_target(build_all_data_packages)

function(create_data_package source_directory_name)
  get_filename_component(source_directory ${source_directory_name} ABSOLUTE)
  set(out_file_name ${TARGET_DESTINATTION}/${source_directory_name}.zip)
  set(package_name ${source_directory_name}.zip)

  file(GLOB_RECURSE package_srcs ${source_directory}/*)
  # CONFIGURE_DEPENDS
  add_custom_command(
    OUTPUT ${out_file_name}
    DEPENDS svfs-cli ${package_srcs}
    COMMAND svfs-cli --action "build_package;${source_directory};${out_file_name}"
    COMMENT "Building package ${package_name}"
    VERBATIM)

  add_custom_target(build_${package_name} DEPENDS svfs-cli ${out_file_name})

  add_dependencies(build_all_data_packages build_${package_name})
endfunction()
