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

# Build libraries and tests:
add_subdirectory(libs)
add_subdirectory(test)

# Add these files to IDE:
add_custom_target(${PROJECT_NAME}-extra-files
  SOURCES
    README.md
    LICENSE
)
