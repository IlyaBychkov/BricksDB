add_library(scan_operator STATIC scan_operator.cpp)
target_include_directories(scan_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)

add_library(aggregation_operator STATIC aggregaiton_operator.cpp)
target_include_directories(aggregation_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(aggregation_operator PUBLIC batch)