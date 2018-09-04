#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(Qt5 COMPONENTS Core REQUIRED)

set(TARGET safl-qt)
add_library(${TARGET}
    include/safl/qt/Executor.h
    include/safl/qt/Futurize.h
    src/safl/qt/Executor.cpp
)
target_include_directories(${TARGET}
  PUBLIC
    include
)
target_link_libraries(${TARGET}
  PUBLIC
    safl
    Qt5::Core
)

safl_use_clang_tidy(${TARGET})

## Unit tests ##

set(TEST_TARGET ut-${TARGET})
add_executable(${TEST_TARGET}
    test/QtTests.cpp
    test/MyObject.h
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    safl-testing
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
