#
# This file is a part of Standalone Future Library (safl).
#

cmake_minimum_required(VERSION 3.3)
project(SAFL CXX)

set(TARGET safl)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(
    -pedantic
    -Werror
    -Wall
    -Wextra
    -Wconversion
    -Wnon-virtual-dtor
    -Wold-style-cast
)

find_package(Qt5 QUIET REQUIRED Core CONFIG NO_CMAKE_PACKAGE_REGISTRY)

add_executable(${TARGET}
    include/safl/detail/FutureDetail.h
    include/safl/Future.h
    include/safl/ToFuture.h
    src/safl/Future.cpp
    test/main.cpp
)

target_include_directories(${TARGET}
  PRIVATE
    include
)

target_link_libraries(${TARGET}
  PRIVATE
    Qt5::Core
)

set_target_properties(${TARGET}
  PROPERTIES
    AUTOMOC ON
)

# add these files to IDE:
add_custom_target(readme_txt SOURCES README.md)
