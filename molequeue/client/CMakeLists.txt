find_package(Qt4 4.8 COMPONENTS QtCore QtNetwork REQUIRED)

include_directories("${MoleQueue_SOURCE_DIR}/thirdparty/qt5json")
set(qt5jljlkson_srcs
  qjsondocument.cpp
  qjsonvalue.cpp
  qjson.cpp
  qjsonobject.cpp
  qjsonwriter.cpp
  qjsonarray.cpp
  qjsonparser.cpp
  )

set(QT5JSON_SOURCE_DIR "${MoleQueue_SOURCE_DIR}/thirdparty/qt5json/")
aux_source_directory(${QT5JSON_SOURCE_DIR} qt5json_srcs)

set(sources
  client.cpp
  job.cpp
  ${qt5json_srcs}
  )

set(headers
  client.h
  job.h
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(MoleQueueClient ${sources} ${jsoncpp_srcs})
set_target_properties(MoleQueueClient PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueClient ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
include(GenerateExportHeader)
generate_export_header(MoleQueueClient EXPORT_FILE_NAME molequeueclientexport.h)
list(APPEND headers "${CMAKE_CURRENT_BINARY_DIR}/molequeueclientexport.h")
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET MoleQueueClient APPEND
    PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

install(FILES ${headers}
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/client")
