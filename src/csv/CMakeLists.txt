add_library(csv_reader STATIC csv_reader.cpp)
target_include_directories(csv_reader PUBLIC ${CMAKE_SOURCE_DIR}/include/csv)

add_library(csv_writer STATIC csv_writer.cpp)
target_include_directories(csv_writer PUBLIC ${CMAKE_SOURCE_DIR}/include/csv)

add_library(csv_batcher STATIC csv_batcher.cpp)
target_include_directories(csv_batcher PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(csv_batcher PUBLIC csv_writer csv_reader batch)
