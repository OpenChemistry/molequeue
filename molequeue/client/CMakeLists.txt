find_package(Qt5 COMPONENTS Network REQUIRED)

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
qt5_use_modules(MoleQueueClient Network)
set_target_properties(MoleQueueClient PROPERTIES AUTOMOC TRUE)
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
