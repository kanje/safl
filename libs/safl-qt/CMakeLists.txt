#
# This file is a part of Stand-alone Future Library (safl).
#

find_package(Qt5 COMPONENTS Core REQUIRED)

set(TARGET safl-qt)
add_library(${TARGET}
    include/safl/qt/Executor.h
    include/safl/qt/Futurize.h
    src/safl/qt/Executor.cpp
)
target_include_directories(${TARGET}
  PUBLIC
    include
)
target_link_libraries(${TARGET}
  PUBLIC
    safl
    Qt5::Core
)
