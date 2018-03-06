#
# This file is a part of Stand-alone Future Library (safl).
#

# Use clang-tidy if available:
find_program(CLANG_TIDY_EXECUTABLE
  NAMES
    clang-tidy
)
if(CLANG_TIDY_EXECUTABLE)
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXECUTABLE}
        -checks=-*,performance*,readability*,misc*,modernize*
    )
endif()

set(TARGET ${PROJECT_NAME})
add_library(${TARGET}
    include/safl/Executor.h
    include/safl/Future.h
    include/safl/ToFuture.h
    include/safl/detail/Context.h
    include/safl/detail/DebugContext.h
    include/safl/detail/ErrorHandling.h
    include/safl/detail/FunctionTraits.h
    include/safl/detail/FutureDetail.h
    include/safl/detail/NonCopyable.h
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
if(SAFL_DEVELOPER)
    target_compile_definitions(${TARGET}
      PUBLIC
        -DSAFL_DEVELOPER
    )
endif()
