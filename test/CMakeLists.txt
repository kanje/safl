#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(GTest REQUIRED)

set(TEST_LIBRARY safl-testing)
add_library(${TEST_LIBRARY}
    src/safl/Testing.cpp
    include/safl/Testing.h
)
target_include_directories(${TEST_LIBRARY}
  PUBLIC
    include
    ${GTEST_INCLUDE_DIRS}
)
target_link_libraries(${TEST_LIBRARY}
  PUBLIC
    ${GTEST_BOTH_LIBRARIES}
    ${PROJECT_NAME}
)

set(TEST_TARGET ut-${PROJECT_NAME})
add_executable(${TEST_TARGET}
    src/SaflTests.cpp
    src/Testing.h
    src/TraitsTests.cpp
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    ${TEST_LIBRARY}
)

gtest_add_tests(
  TARGET
    ${TEST_TARGET}
  TEST_PREFIX
    ${PROJECT_NAME}.
)
