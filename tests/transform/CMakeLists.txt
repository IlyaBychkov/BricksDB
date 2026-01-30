add_executable(csv_to_columnar_test csv_to_columnar_test.cpp)
target_include_directories(csv_to_columnar_test PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(csv_to_columnar_test csv_to_columnar)

add_executable(columnar_to_csv_test columnar_to_csv_test.cpp)
target_include_directories(columnar_to_csv_test PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(columnar_to_csv_test columnar_to_csv)

add_executable(transform_test transform_test.cpp)
target_include_directories(transform_test PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(transform_test csv_to_columnar columnar_to_csv GTest::gtest GTest::gtest_main)
add_test(NAME transform_test COMMAND transform_test)