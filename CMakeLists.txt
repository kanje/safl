#
# This file is a part of Stand-alone Future Library (safl).
#

cmake_minimum_required(VERSION 3.9)
project(safl VERSION 0.3 LANGUAGES CXX)

option(SAFL_DEVELOPER "Enable the developer mode" OFF)

if(SAFL_DEVELOPER)
    add_definitions(-DSAFL_DEVELOPER)
endif()

# Check if we are embedded into a source tree of some other project:
if("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
    set(SAFL_IS_ROOT_PROJECT TRUE)
endif()

# Documentation:
add_subdirectory(doc)

# Enable unit-testing:
enable_testing()

# Warning options:
add_compile_options(
    -pedantic
    -Werror
    -Wall
    -Wextra
    -Wconversion
    -Wnon-virtual-dtor
    -Wold-style-cast
)

# Build libraries:
add_subdirectory(libs)

# Add these files to IDE:
add_custom_target(${PROJECT_NAME}-extra-files
  SOURCES
    README.md
    LICENSE
)

# Installation:
include(CMakePackageConfigHelpers)

set(SAFL_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/safl-${PROJECT_VERSION})
set(SAFL_CMAKE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cmake)
set(SAFL_VERSION_CMAKE ${SAFL_CMAKE_DIR}/SaflConfigVersion.cmake)
set(SAFL_CONFIG_CMAKE ${SAFL_CMAKE_DIR}/SaflConfig.cmake)

write_basic_package_version_file(
    ${SAFL_VERSION_CMAKE} COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/ProjectConfig.cmake.in
    ${SAFL_CONFIG_CMAKE}
  INSTALL_DESTINATION
    ${SAFL_INSTALL_CMAKE_DIR}
)

export(
  EXPORT
    SaflTargets
  FILE
    ${SAFL_CMAKE_DIR}/SaflTargets.cmake
)
install(
  FILES
    ${SAFL_VERSION_CMAKE}
    ${SAFL_CONFIG_CMAKE}
  DESTINATION
    ${SAFL_INSTALL_CMAKE_DIR}
)
install(
  EXPORT
    SaflTargets
  DESTINATION
    ${SAFL_INSTALL_CMAKE_DIR}
)
