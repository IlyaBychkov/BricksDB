add_library(type STATIC type.cpp)
target_include_directories(type PUBLIC ${CMAKE_SOURCE_DIR}/include/)

add_library(schema STATIC schema.cpp)
target_include_directories(schema PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(schema PUBLIC type csv_reader csv_writer)

add_library(column STATIC column.cpp)
target_include_directories(column PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(column PUBLIC type)

add_library(batch STATIC batch.cpp)
target_include_directories(batch PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(batch PUBLIC column schema)
