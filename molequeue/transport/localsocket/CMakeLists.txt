set(mq_localsocket_connection_srcs
  localsocketconnection.cpp)

set(mq_localsocket_connectionlistener_srcs
  localsocketconnectionlistener.cpp)

set(mq_client_localsocket_srcs
  localsocketclient.cpp)

add_library(mqlocalsocketconnection STATIC localsocketconnection.cpp)
set_target_properties(mqlocalsocketconnection PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketconnection
  mqconnection
  ${QT_LIBRARIES})

add_library(mqlocalsocketconnectionlistener STATIC
  localsocketconnectionlistener.cpp)
set_target_properties(mqlocalsocketconnectionlistener PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketconnectionlistener
  mqconnectionlistener
  mqlocalsocketconnection
  ${QT_LIBRARIES})

# local socket client
add_library(mqlocalsocketclient localsocketclient.cpp)
set_target_properties(mqlocalsocketclient PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketclient
  molequeueclient
  mqlocalsocketconnection)

set(hdrs localsocketclient.h ${CMAKE_CURRENT_BINARY_DIR}/mqlocalsocketclientexport.h)

generate_export_header(mqlocalsocketclient EXPORT_FILE_NAME mqlocalsocketclientexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET mqlocalsocketclient APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

list(APPEND hdrs
  localsocketconnection.h
  localsocketconnectionlistener.h
  )

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/transport")
install(
  TARGETS
    mqlocalsocketconnection
    mqlocalsocketclient
    mqlocalsocketconnectionlistener
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )
add_subdirectory(plugin)
