add_library(csv_to_columnar STATIC csv_to_columnar.cpp)
target_include_directories(csv_to_columnar PUBLIC ${CMAKE_SOURCE_DIR}/include/)

add_library(metadata STATIC metadata.cpp)
target_include_directories(metadata PUBLIC ${CMAKE_SOURCE_DIR}/include/)