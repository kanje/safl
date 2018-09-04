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

## safl tests ##

set(TEST_TARGET ut-safl)
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

## safl-qt tests ##

find_package(Qt5 COMPONENTS Core)

set(TEST_TARGET ut-safl-qt)
add_executable(${TEST_TARGET}
    src/QtTests.cpp
    src/Testing.h
    src/MyObject.h
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    ${TEST_LIBRARY}
    safl-qt
)
set_target_properties(${TEST_TARGET}
  PROPERTIES
    AUTOMOC ON
)

gtest_add_tests(
  TARGET
    ${TEST_TARGET}
  TEST_PREFIX
    ${PROJECT_NAME}.
)
