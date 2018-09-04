#
# This file is a part of Stand-alone Future Library (safl).
#

# Use clang-tidy if available:
find_program(CLANG_TIDY_EXECUTABLE
  NAMES
    clang-tidy
)
macro(safl_use_clang_tidy TARGET)
    if(CLANG_TIDY_EXECUTABLE)
        set(CLANG_TIDY_COMMAND ${CLANG_TIDY_EXECUTABLE}
                          -checks=-*,performance*,readability*,misc*,modernize*)
        set_target_properties(${TARGET}
          PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )
    endif()
endmacro()

# Build libraries:
add_subdirectory(testing)
add_subdirectory(core)
add_subdirectory(extensions)
