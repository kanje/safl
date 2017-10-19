#
# This file is a part of Stand-alone Future Library (safl).
#

cmake_minimum_required(VERSION 3.9)
project(safl VERSION 0.2 LANGUAGES CXX)

option(SAFL_DEVELOPER "Enable the developer mode" ON)

add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(test)

# add these files to IDE:
add_custom_target(${PROJECT_NAME}-extra-files
  SOURCES
    README.md
    LICENSE
)
