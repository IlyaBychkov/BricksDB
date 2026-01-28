add_library(csv_reader STATIC csv_reader.cpp)
target_include_directories(csv_reader PUBLIC ${CMAKE_SOURCE_DIR}/include/csv)

add_library(csv_writer STATIC csv_writer.cpp)
target_include_directories(csv_writer PUBLIC ${CMAKE_SOURCE_DIR}/include/csv)