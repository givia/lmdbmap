cmake_minimum_required(VERSION 3.14)
project(lmdbmap)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to Release as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS serialization system filesystem)

find_library(LMDB_LIBRARY lmdb)
find_path(LMDB_INCLUDE_DIR lmdb.h)

if (NOT LMDB_LIBRARY OR NOT LMDB_INCLUDE_DIR)
    message(FATAL_ERROR "LMDB not found")
endif()

include_directories(${LMDB_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} include)

add_library(lmdbmap INTERFACE)
target_include_directories(lmdbmap INTERFACE include)
target_link_libraries(lmdbmap INTERFACE ${LMDB_LIBRARY} ${Boost_LIBRARIES})

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(benchmarks)
