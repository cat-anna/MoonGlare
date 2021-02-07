add_custom_target(build_all_libs)
add_custom_target(build_all_test)
add_custom_target(execute_all_test)

function(define_static_lib target_name)

  file(GLOB_RECURSE SRC src/*.cpp src/*.hpp include/*.hpp)
  list(FILTER SRC EXCLUDE REGEX ".*_(g|catch)test\..*")

  add_library(${target_name} STATIC ${SRC})
  target_include_directories(${target_name} PUBLIC include)
  target_include_directories(${target_name} PRIVATE src)
  add_dependencies(build_all_libs ${target_name})

  set(TARGET
      ${target_name}
      PARENT_SCOPE)
endfunction()

function(define_ut_target target_name ut_target_name target_src)
  add_executable(${ut_target_name} ${target_src})
  target_include_directories(${ut_target_name} PRIVATE src test)
  target_link_libraries(${ut_target_name} PUBLIC ${target_name})
  add_dependencies(build_all_test ${ut_target_name})
  add_custom_target(
    run_${ut_target_name}
    COMMAND ${ut_target_name}
    WORKING_DIRECTORY ${TARGET_DESTINATTION}
    COMMENT "Running test ${ut_target_name}"
    DEPENDS ${ut_target_name} ${target_name})
  add_dependencies(execute_all_test run_${ut_target_name})
endfunction()

function(define_static_lib_with_ut target_name)
  define_static_lib(${target_name})

  file(GLOB_RECURSE gtest_src test/*_gtest.cpp src/*_gtest.cpp)
  if(gtest_src)
    set(ut_gtest_target ut_gtest_${target_name})
    define_ut_target(${target_name} ${ut_gtest_target} "${gtest_src}")
    target_link_libraries(${ut_gtest_target} PUBLIC gtest_main)
    target_compile_definitions(${ut_gtest_target} PRIVATE -DWANTS_GTEST_MOCKS)
  endif()

  file(GLOB_RECURSE catchtest_src test/* src/*_catchtest.cpp)
  if(catchtest_src)
    set(ut_catchtest_target ut_catchtest_${target_name})
    define_ut_target(${target_name} ${ut_catchtest_target} "${catchtest_src}")
    target_link_libraries(${ut_catchtest_target} PUBLIC catchtest_main)
    # target_compile_definitions(${ut_catchtest_target} PRIVATE -DWANTS_GTEST_MOCKS) #??
  endif()

  set(TARGET
      ${target_name}
      PARENT_SCOPE)
  set(TARGET_UT_GTEST
      ${ut_gtest_target}
      PARENT_SCOPE)
  set(TARGET_UT_CATCHTEST
      ${ut_catchtest_target}
      PARENT_SCOPE)
endfunction()
