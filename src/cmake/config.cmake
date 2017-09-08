set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "build type")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(ASSOCIATIVE_DEBUG true)
else("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(ASSOCIATIVE_DEBUG false)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

# should be a number, not a string
set(ASSOCIATIVE_MAX_LOCK_TIME 10 CACHE STRING "maximum wait time for shared memory locks in seconds")

# should be the name of an isolation level in env/isolation_impl
set(ASSOCIATIVE_DEFAULT_ISOLEVEL "almost-full" CACHE STRING "default isolation level to use")

# should be a path
set(ASSOCIATIVE_DEFAULT_LOG "/var/local/log/associative-fs" CACHE FILEPATH "default log file")

set(ASSOCIATIVE_WITH_SQLITE true CACHE BOOL "build with SQLite support")
set(ASSOCIATIVE_WITH_MYSQL true CACHE BOOL "build with MySQL support")

configure_file(cmake/config.hpp.in gen/config.hpp)
