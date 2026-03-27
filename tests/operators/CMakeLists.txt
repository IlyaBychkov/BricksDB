add_executable(clickbench clickbench.cpp)
target_include_directories(clickbench PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(clickbench scan_operator aggregation_operator csv_writer metadata column scheme GTest::gtest GTest::gtest_main)
add_test(NAME clickbench COMMAND clickbench)
