# These classes were taken from the Qt5 repository and ported to work with
# Qt 4.8. As such they are bound by the same licenses as Qt 5. No changes
# were made to any public API, and so they should be source compatible with
# the Qt 5 JSON classes.

project(Qt5Json)
cmake_minimum_required(VERSION 2.8.6)

find_package(Qt4 4.8 REQUIRED QtCore)
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

if(NOT QT5_JSON_LIBRARY_NAME)
  set(QT5_JSON_LIBRARY_NAME Qt5Json)
endif()

add_library(${QT5_JSON_LIBRARY_NAME} ${sources})
target_link_libraries(${QT5_JSON_LIBRARY_NAME} ${QT_QTCORE_LIBRARY})
