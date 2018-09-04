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

# Build the main library:
add_subdirectory(safl)

# Build extensions:
function(add_extension SUBDIR EXT_NAME SUMMARY ENABLED_BY_DEFAULT)
    set(EXT_OPTION SAFL_ENABLE_${EXT_NAME})
    option(${EXT_OPTION} "Enable the ${SUMMARY}" ${ENABLED_BY_DEFAULT})
    if(${EXT_OPTION})
        add_subdirectory(${SUBDIR})
    endif()
endfunction()

add_extension(safl-qt QT "integration with Qt" ON)
