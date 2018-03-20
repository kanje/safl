#
# This file is a part of Stand-alone Future Library (safl).
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
if(QHELPGENERATOR_EXECUTABLE)
    set(SAFL_DOXY_GENERATE_QHP YES)
    set(SAFL_DOXY_QHG_LOCATION ${QHELPGENERATOR_EXECUTABLE})
endif()

if(SAFL_DEVELOPER)
    set(SAFL_DOXY_INTERNAL_DOCS YES)
endif()

# If safl is embedded into some other project, that project could already have
# a "make doc" target, so in order to avoid name clashes we name it "safl-doc".
set(SAFL_DOXY_TARGET doc)
if(NOT SAFL_IS_ROOT_PROJECT)
    set(SAFL_DOXY_TARGET safl-doc)
endif()

configure_file(Doxyfile.in Doxyfile @ONLY)
add_custom_target(${SAFL_DOXY_TARGET}
  COMMAND
    ${DOXYGEN_EXECUTABLE} Doxyfile
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES
    style/CustomDoxygen.css
    style/DoxygenLayout.xml
    style/Header.html
    pages/0_MainPage.dox
  COMMENT
    "Generating SAFL documentation"
  VERBATIM
)
