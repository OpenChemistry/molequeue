# These classes were taken from the Qt5 repository and ported to work with
# Qt 4.8. As such they are bound by the same licenses as Qt 5. No changes
# were made to any public API, and so they should be source compatible with
# the Qt 5 JSON classes.

project(Qt5Json)
cmake_minimum_required(VERSION 2.8.6)

find_package(Qt4 4.8 REQUIRED QtCore)
include(${QT_USE_FILE})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

set(sources
  qjson.cpp
  qjsondocument.cpp
  qjsonparser.cpp
  qjsonwriter.cpp
  qjsonarray.cpp
  qjsonobject.cpp
  qjsonvalue.cpp
  )

set(headers
  qjsonarray.h
  qjsondocument.h
  qjsonobject.h
  qjsonvalue.h
  ${CMAKE_CURRENT_BINARY_DIR}/qt5jsonexport.h
  )

# Macro to only set if not set...
macro(set_if_not_set _var_name)
  if(NOT DEFINED ${_var_name})
    set(${_var_name} ${ARGN})
  endif()
endmacro()

# Set up a few defaults if the values were not already set.
set_if_not_set(QT5_JSON_LIBRARY_NAME Qt5Json)
set_if_not_set(INSTALL_RUNTIME_DIR bin)
set_if_not_set(INSTALL_LIBRARY_DIR lib)
set_if_not_set(INSTALL_ARCHIVE_DIR lib)
set_if_not_set(INSTALL_INCLUDE_DIR include)
set_if_not_set(EXPORT_TARGETS_NAME Qt5JsonTargets)

add_library(${QT5_JSON_LIBRARY_NAME} ${sources})
target_link_libraries(${QT5_JSON_LIBRARY_NAME} ${QT_QTCORE_LIBRARY})

include(GenerateExportHeader)

generate_export_header(${QT5_JSON_LIBRARY_NAME}
  EXPORT_FILE_NAME qt5jsonexport.h
  EXPORT_MACRO_NAME QT5JSON_EXPORT)

set_property(TARGET ${QT5_JSON_LIBRARY_NAME} APPEND
  PROPERTY COMPILE_FLAGS ${qt5json_export_flags})

# Now to install the appropriate files.
install(FILES ${headers} DESTINATION "${INSTALL_INCLUDE_DIR}/qt5json")
install(TARGETS ${QT5_JSON_LIBRARY_NAME}
  EXPORT ${EXPORT_TARGETS_NAME}
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )
