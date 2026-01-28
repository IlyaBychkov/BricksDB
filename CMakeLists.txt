cmake_minimum_required(VERSION 3.22)
project(BricksDB)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

include(cmake/dependencies_package.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function")

add_subdirectory(src)
add_subdirectory(tests)
