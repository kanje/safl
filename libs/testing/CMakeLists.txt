#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(GTest REQUIRED)

set(TARGET safl-testing)
add_library(${TARGET}
    include/safl/testing/Testing.h
    src/safl/testing/Testing.cpp
)
target_include_directories(${TARGET}
  PUBLIC
    include
    ${GTEST_INCLUDE_DIRS}
)
target_link_libraries(${TARGET}
  PUBLIC
    ${GTEST_BOTH_LIBRARIES}
    safl
)
