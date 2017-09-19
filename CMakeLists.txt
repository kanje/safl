#
# This file is a part of Stand-alone Future Library (safl).
#

cmake_minimum_required(VERSION 3.3)
project(SAFL VERSION 0.1 LANGUAGES CXX C)

option(SAFL_DEVELOPER "Enable the developer mode" ON)

if(SAFL_DEVELOPER)
    add_compile_options(
        -pedantic
        -Werror
        -Wall
        -Wextra
        -Wconversion
        -Wnon-virtual-dtor
        -Wold-style-cast
    )
endif()

enable_testing()

add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(test)

# add these files to IDE:
add_custom_target(safl-extra-files
  SOURCES
    README.md
    LICENSE
)
