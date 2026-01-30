add_library(metadata STATIC metadata.cpp)
target_include_directories(metadata PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(metadata PUBLIC scheme)

add_library(csv_to_columnar STATIC csv_to_columnar.cpp)
target_include_directories(csv_to_columnar PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(csv_to_columnar PUBLIC csv_batcher metadata)

add_library(columnar_to_csv STATIC columnar_to_csv.cpp)
target_include_directories(columnar_to_csv PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(columnar_to_csv PUBLIC csv_writer batch metadata)
