#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

set(TEST_TARGET ut-safl)
add_executable(${TEST_TARGET}
    src/SaflTests.cpp
    src/TraitsTests.cpp
    src/Testing.h
)
target_include_directories(${TEST_TARGET}
  PRIVATE
    ${GTEST_INCLUDE_DIRS}
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    Threads::Threads
    safl
)

# Tests for the CommonAPI extension:
if(SAFL_EXT_ENABLE_CAPI)
    target_sources(${TEST_TARGET}
      PRIVATE
        src/CapiTests.cpp
    )
    target_link_libraries(${TEST_TARGET}
      PRIVATE
        safl-capi
    )
endif()

GTEST_ADD_TESTS(${TEST_TARGET} "" AUTO)
