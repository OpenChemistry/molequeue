find_package(Qt4 4.8 COMPONENTS QtCore QtNetwork REQUIRED)
include_directories(SYSTEM ${QT_INCLUDES})
add_definitions(${QT_DEFINITIONS})

set(sources
  client.cpp
  jobobject.cpp
  jsonrpcclient.cpp
  )

set(headers
  client.h
  jobobject.h
  jsonrpcclient.h
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(MoleQueueClient ${sources})
set_target_properties(MoleQueueClient PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueClient
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  Qt5Json)
include(GenerateExportHeader)
generate_export_header(MoleQueueClient EXPORT_FILE_NAME molequeueclientexport.h)
list(APPEND headers "${CMAKE_CURRENT_BINARY_DIR}/molequeueclientexport.h")
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET MoleQueueClient APPEND
    PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

install(FILES ${headers}
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/client")
install(TARGETS MoleQueueClient
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )
