set(TARGET gtest_main)
file(GLOB_RECURSE SRC_GTEST src/gtest_*)
add_library(${TARGET} STATIC ${SRC_GTEST})
target_link_libraries(${TARGET} PUBLIC base fmt::fmt orbit_logger GTest::gmock GTest::gtest)

set(TARGET catchtest_main)
file(GLOB_RECURSE SRC_GTEST src/catchtest_*)
add_library(${TARGET} STATIC ${SRC_GTEST})
target_link_libraries(${TARGET} PUBLIC base fmt::fmt orbit_logger)
