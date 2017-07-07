#
# This file is a part of Standalone Future Library (safl).
#

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    return()
endif()

find_program(QHELPGENERATOR_EXECUTABLE
  NAMES
    qhelpgenerator
    qhelpgenerator-qt5
)
if(NOT QHELPGENERATOR_EXECUTABLE)
    unset(QHELPGENERATOR_EXECUTABLE CACHE)
endif()


configure_file(Doxyfile.in Doxyfile @ONLY)
add_custom_target(doc
  COMMAND
    ${DOXYGEN_EXECUTABLE} Doxyfile
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT
    "Generating SAFL documentation"
  VERBATIM
)
