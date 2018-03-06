#
# This file is a part of Stand-alone Future Library (safl).
#

cmake_minimum_required(VERSION 3.9)
project(safl VERSION 0.2 LANGUAGES CXX)

option(SAFL_DEVELOPER "Enable the developer mode" OFF)

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

# Build the library and tests:
add_subdirectory(lib)
add_subdirectory(test)

# Add these files to IDE:
add_custom_target(${PROJECT_NAME}-extra-files
  SOURCES
    README.md
    LICENSE
)
