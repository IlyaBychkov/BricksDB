add_executable(clickbench clickbench.cpp)
target_include_directories(clickbench PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(clickbench scan_operator aggregation_operator csv_writer metadata column scheme GTest::gtest GTest::gtest_main filter_operator sort_operator limit_operator time_transform map_operator)
add_test(NAME clickbench COMMAND clickbench)

add_executable(compare_csv compare_csv.cpp)
target_link_libraries(compare_csv GTest::gtest GTest::gtest_main)
add_test(NAME compare_csv COMMAND compare_csv)