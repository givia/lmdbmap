cmake_minimum_required(VERSION 3.14)
project(lmdbmap VERSION 0.3.0)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to Release as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Boost REQUIRED COMPONENTS serialization system filesystem)

find_library(LMDB_LIBRARY lmdb)
find_path(LMDB_INCLUDE_DIR lmdb.h)

if (NOT LMDB_LIBRARY OR NOT LMDB_INCLUDE_DIR)
    message(FATAL_ERROR "LMDB not found")
endif()

# Create an imported target for LMDB to make exporting cleaner
add_library(LMDB::LMDB UNKNOWN IMPORTED)
set_target_properties(LMDB::LMDB PROPERTIES
    IMPORTED_LOCATION "${LMDB_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LMDB_INCLUDE_DIR}"
)

add_library(lmdbmap INTERFACE)
target_include_directories(lmdbmap INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(lmdbmap INTERFACE LMDB::LMDB ${Boost_LIBRARIES})

# Installation
install(DIRECTORY include/lmdbmap DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS lmdbmap
    EXPORT lmdbmapTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT lmdbmapTargets
    FILE lmdbmapTargets.cmake
    NAMESPACE lmdbmap::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lmdbmap
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lmdbmapConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lmdbmapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lmdbmapConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lmdbmap
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/lmdbmapConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lmdbmapConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lmdbmap
)

# CPack
set(CPACK_PACKAGE_NAME "lmdbmap")
set(CPACK_PACKAGE_VENDOR "Ali Javadi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ wrapper for LMDB providing std::map like interface")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ali Javadi")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "liblmdb-dev, libboost-serialization-dev, libboost-system-dev, libboost-filesystem-dev")

include(CPack)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(benchmarks)
