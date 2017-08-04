#
# This file is a part of Stand-alone Future Library (safl).
#

set(TARGET safl)

add_library(${TARGET}
    include/safl/detail/Context.h
    include/safl/detail/ErrorHandling.h
    include/safl/detail/FunctionTraits.h
    include/safl/detail/FutureDetail.h
    include/safl/detail/Utils.h
    include/safl/Executor.h
    include/safl/Future.h
    include/safl/ToFuture.h
    src/safl/Future.cpp
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

# Extensions:
add_subdirectory(ext)
