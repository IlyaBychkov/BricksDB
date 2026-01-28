add_library(scheme STATIC scheme.cpp)
target_include_directories(scheme PUBLIC ${CMAKE_SOURCE_DIR}/include/)

add_library(type STATIC type.cpp)
target_include_directories(type PUBLIC ${CMAKE_SOURCE_DIR}/include/scheme)