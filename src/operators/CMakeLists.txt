add_library(scan_operator STATIC scan_operator.cpp)
target_include_directories(scan_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)

add_library(aggregation_operator STATIC aggregation_operator.cpp)
target_include_directories(aggregation_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(aggregation_operator PUBLIC batch)

add_library(filter_operator STATIC filter_operator.cpp)
target_include_directories(filter_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(filter_operator PUBLIC batch)

add_library(sort_operator STATIC sort_operator.cpp)
target_include_directories(sort_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(sort_operator PUBLIC batch)

add_library(limit_operator STATIC limit_operator.cpp)
target_include_directories(limit_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(limit_operator PUBLIC batch)

add_library(map_operator STATIC map_operator.cpp)
target_include_directories(map_operator PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(map_operator PUBLIC batch expressions)