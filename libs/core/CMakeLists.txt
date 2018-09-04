#
# This file is a part of Stand-alone Future Library (safl).
#

set(TARGET safl)
add_library(${TARGET}
    include/safl/Composition.h
    include/safl/Executor.h
    include/safl/Future.h
    include/safl/ToFuture.h
    include/safl/detail/Context.h
    include/safl/detail/DebugContext.h
    include/safl/detail/FunctionTraits.h
    include/safl/detail/FutureDetail.h
    include/safl/detail/NonCopyable.h
    include/safl/detail/Signalling.h
    include/safl/detail/TypeEraser.h
    include/safl/detail/UniqueInstance.h
    src/safl/Executor.cpp
    src/safl/detail/Context.cpp
    src/safl/detail/DebugContext.cpp
)
target_include_directories(${TARGET}
  PUBLIC
    include
)
target_compile_features(${TARGET}
  PUBLIC
    cxx_lambda_init_captures # this enforces c++14 or later
)

safl_use_clang_tidy(${TARGET})

## Unit tests ##

set(TEST_TARGET ut-${TARGET})
add_executable(${TEST_TARGET}
    test/CoreTests.cpp
    test/TraitsTests.cpp
)
target_link_libraries(${TEST_TARGET}
  PRIVATE
    safl-testing
)

gtest_add_tests(
  TARGET
    ${TEST_TARGET}
  TEST_PREFIX
    ${PROJECT_NAME}.
)
