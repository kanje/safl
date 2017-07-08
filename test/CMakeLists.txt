#
# This file is a part of Stand-alone Future Library (safl).
#

set(TEST_TARGET ut-safl)

add_executable(${TEST_TARGET}
    src/Tests.cpp
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    gtest_main
    gtest
    safl
)
add_test(SaflTest ${TEST_TARGET})
