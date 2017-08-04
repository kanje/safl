#
# This file is a part of Stand-alone Future Library (safl).
#

function(add_extension SUBDIR SUMMARY ENABLED_BY_DEFAULT)
    string(TOUPPER ${SUBDIR} EXT_NAME)
    set(EXT_OPTION SAFL_EXT_ENABLE_${EXT_NAME})
    option(${EXT_OPTION} "Enable the ${SUMMARY} extension" ${ENABLED_BY_DEFAULT})
    if(${EXT_OPTION})
        add_subdirectory(${SUBDIR})
    endif()
endfunction()

add_extension(capi "CommonAPI" OFF)
add_extension(qt "Qt EventLoop" OFF)
