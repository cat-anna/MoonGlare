subdirlist(package_list ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${package_list})
  create_data_package(${subdir})
endforeach()
