project(associative-fs)

cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

include(cmake/src.cmake)
include(cmake/config.cmake)
include(cmake/sqlite.cmake)
include(cmake/mysql.cmake)

set(CORE_LIBS boost_filesystem boost_program_options log4cpp ${OPT_LIBS})

# Core
add_library(fs-core STATIC ${CORE_SRCS})

target_link_libraries(fs-core ${CORE_LIBS})

# Main
add_executable(fs-main main.cpp ${MAIN_SRCS})

target_link_libraries(fs-main fs-core)

# Test
add_executable(fs-test ${TEST_SRCS})

target_link_libraries(fs-test fs-core gtest)

# General settings
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
