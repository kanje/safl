#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(CommonAPI REQUIRED CONFIG)
set_target_properties(CommonAPI
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${COMMONAPI_INCLUDE_DIRS}
)

set(EXTENSION ${TARGET}-capi)
add_library(${EXTENSION}
    include/safl/capi/ToFuture.h
)
target_include_directories(${EXTENSION}
  PUBLIC
    include
)
target_link_libraries(${EXTENSION}
  PUBLIC
    ${TARGET}
    CommonAPI
)
