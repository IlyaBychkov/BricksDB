add_library(scan_operator STATIC scan_operator.cpp)
target_include_directories(scan_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
# target_link_libraries(csv_batcher PUBLIC csv_writer csv_reader batch)
