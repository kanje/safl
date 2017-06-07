#
# This file is a part of Standalone Future Library (safl).
#

cmake_minimum_required(VERSION 3.3)
project(SAFL CXX)

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

# library:
set(TARGET safl)

add_library(${TARGET}
    include/safl/detail/FunctionTraits.h
    include/safl/detail/FutureDetail.h
    include/safl/Future.h
    include/safl/ToFuture.h
    src/safl/Future.cpp
)
target_include_directories(${TARGET}
  PUBLIC
    include
)

# unit tests:
enable_testing()
set(TEST_TARGET ut-${TARGET})

add_executable(${TEST_TARGET}
    test/Tests.cpp
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    gtest_main
    gtest
    safl
)
add_test(SaflTest ${TEST_TARGET})

# add these files to IDE:
add_custom_target(readme_txt SOURCES README.md)
