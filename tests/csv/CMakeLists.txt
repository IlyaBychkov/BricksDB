add_executable(csv_reader_test csv_reader_test.cpp)
target_include_directories(csv_reader_test PUBLIC ${CMAKE_SOURCE_DIR}/include/csv)
target_link_libraries(csv_reader_test csv_reader GTest::gtest GTest::gtest_main)

add_test(NAME csv_reader_test COMMAND csv_reader_test)