#
# This file is a part of Stand-alone Future Library (safl).
#

include(GNUInstallDirs)

# Use standard C++:
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable unit-testing:
enable_testing()

# Warning options:
add_compile_options(
    -pedantic
    -Werror
    -Wall
    -Wextra
    -Wconversion
    -Wnon-virtual-dtor
    -Wold-style-cast
)

find_program(CLANG_TIDY_EXECUTABLE
  NAMES
    clang-tidy
)

function(safl_configure_target TARGET)
    # Use clang-tidy if available:
    if(CLANG_TIDY_EXECUTABLE)
        set(CLANG_TIDY_COMMAND ${CLANG_TIDY_EXECUTABLE}
                          -checks=-*,performance*,readability*,misc*,modernize*)
        set_target_properties(${TARGET}
          PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )
    endif()

    set(SAFL_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/safl-${PROJECT_VERSION}/${TARGET})

    target_include_directories(${TARGET}
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
      INTERFACE
        $<INSTALL_INTERFACE:${SAFL_INSTALL_INCLUDEDIR}>
    )

    target_compile_features(${TARGET}
      PUBLIC
        cxx_lambda_init_captures # this enforces c++14 or later
    )

    install(
      TARGETS
        ${TARGET}
      EXPORT
        SaflTargets
      ARCHIVE DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/safl-${PROJECT_VERSION}
    )
    install(
      DIRECTORY
        include/
      DESTINATION
        ${SAFL_INSTALL_INCLUDEDIR}
    )
endfunction()

# Build libraries:
add_subdirectory(testing)
add_subdirectory(core)
add_subdirectory(extensions)
