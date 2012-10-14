# These classes were taken from the Qt5 repository and ported to work with
# Qt 4.8. As such they are bound by the same licenses as Qt 5. No changes
# were made to any public API, and so they should be source compatible with
# the Qt 5 JSON classes.

project(Qt5Json)
cmake_minimum_required(VERSION 2.8.6)

find_package(Qt4 4.8 REQUIRED)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(sources
  qjson.cpp
  qjsondocument.cpp
  qjsonparser.cpp
  qjsonwriter.cpp
  qjsonarray.cpp
  qjsonobject.cpp
  qjsonvalue.cpp
  )

add_library(Qt5Json ${sources})
target_link_libraries(Qt5Json ${QT_QTCORE_LIBRARY})

# Verifies basic functionality, not implemented as a proper test yet.
if(BUILD_LAME_TEST_EXECUTABLE)
  add_executable(json main.cpp)
  target_link_libraries(json Qt5Json)
endif()
