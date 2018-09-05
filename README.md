# SAFL
<b>S</b>tand-<b>a</b>lone <b>F</b>uture <b>L</b>ibrary, or _safl_, is a C++14
library which provides builing blocks for developing an asynchronous software using
the Future/Promise pattern.

<b>The library is under development. It is not yet usable for a production code.</b>

## Background
A concept behind the library is inspired by <a href="https://github.com/facebook/
folly/blob/master/folly/docs/Futures.md">Folly Futures</a>. But, in contrast to it,
safl is not a part of any framework and thus depends only on the C++ standard library.

Besides that, there are several differences in a design of the library. The most
notable one --- no C++ exceptions.

## Libraries
Safl consists of a core library and its extensions. The core library provides a
generic, framework independant inplementation of the Future/Promise pattern. The
extensions, on the other hand, provide an integration with specific frameworks
or libraries, e.g. with <a href="https://www.qt.io/">Qt</a> or <a href="https://
genivi.github.io/capicxx-core-tools/">CommonAPI</a>.

## Build and Install
Safl build system is based on <a href="https://cmake.org/cmake/help/latest/">CMake</a>
and follows the default `cmake`, `make` and `make install` pattern.

The documentation can be built with `make doc`. This step requires <a href="http://
www.stack.nl/~dimitri/doxygen/index.html">Doxygen</a>.

## Getting Started
A project can start using safl by adding this line to its `CMakeLists.txt`:
```
find_package(Safl)
```
and linking to `safl` or `safl-qt` libraries:
```
target_link_libraries(${TARGET} safl)
```
