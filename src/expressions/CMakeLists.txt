add_library(expressions STATIC expressions.cpp)
target_include_directories(expressions PUBLIC ${CMAKE_SOURCE_DIR}/include/expressions)
target_link_libraries(expressions PUBLIC batch)