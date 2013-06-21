find_package(Qt4 4.8 COMPONENTS QtCore QtNetwork REQUIRED)
include_directories(SYSTEM ${QT_INCLUDES})
add_definitions(${QT_DEFINITIONS})

include(GenerateExportHeader)

add_library(MoleQueueServerCore
  connection.h
  connectionlistener.h
  jsonrpc.cpp
  localsocketconnection.cpp
  localsocketconnectionlistener.cpp
  message.cpp
  messageidmanager_p.cpp
)

set_target_properties(MoleQueueServerCore PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueServerCore
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  Qt5Json)

set(hdrs
  connection.h
  connectionlistener.h
  connectionlistenerfactory.h
  jsonrpc.h
  localsocketconnection.h
  localsocketconnectionlistener.h
  message.h
  servercoreglobal.h
)

generate_export_header(MoleQueueServerCore
  EXPORT_FILE_NAME molequeueservercoreexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET MoleQueueServerCore APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

list(APPEND hdrs
  "${CMAKE_CURRENT_BINARY_DIR}/molequeueservercoreexport.h")

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/servercore")
install(TARGETS MoleQueueServerCore
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})
