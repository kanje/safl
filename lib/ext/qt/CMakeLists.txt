#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(Qt5 REQUIRED COMPONENTS Core CONFIG)

set(EXTENSION ${TARGET}-qt)
add_library(${EXTENSION}
    include/safl/qt/Executor.h
    src/safl/qt/Executor.cpp
)
target_include_directories(${EXTENSION}
  PUBLIC
    include
)
target_link_libraries(${EXTENSION}
  PUBLIC
    ${TARGET}
    Qt5::Core
)
