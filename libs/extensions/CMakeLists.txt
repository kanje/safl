#
# This file is a part of Stand-alone Future Library (safl).
#

function(safl_extension SUBDIR SUMMARY ENABLED_BY_DEFAULT)
    string(TOUPPER ${SUBDIR} EXT_NAME)
    set(EXT_OPTION SAFL_ENABLE_${EXT_NAME})
    option(${EXT_OPTION} "Enable the ${SUMMARY}" ${ENABLED_BY_DEFAULT})
    if(${EXT_OPTION})
        add_subdirectory(${SUBDIR})
    endif()
endfunction()

safl_extension(qt "integration with Qt" ON)
