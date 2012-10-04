# Connection
set(mq_localsocket_connection_srcs
  localsocketconnection.cpp
  ${jsoncpp_srcs}
)

add_library(mqlocalsocketconnection STATIC ${mq_localsocket_connection_srcs})
list(APPEND localsocket_targets mqlocalsocketconnection)
list(APPEND localsocket_headers localsocketconnection.h)
set_target_properties(mqlocalsocketconnection PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketconnection
  mqconnection
  ${QT_LIBRARIES}
)

# Connection Listener
set(mq_localsocket_connectionlistener_srcs
  localsocketconnectionlistener.cpp
)

add_library(mqlocalsocketconnectionlistener STATIC
  ${mq_localsocket_connectionlistener_srcs}
)
list(APPEND localsocket_targets mqlocalsocketconnectionlistener)
list(APPEND localsocket_headers localsocketconnectionlistener.h)
set_target_properties(mqlocalsocketconnectionlistener PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketconnectionlistener
  mqconnectionlistener
  mqlocalsocketconnection
  ${QT_LIBRARIES}
)

# Client library
if(MQ_BUILD_CLIENT)
  set(mq_client_localsocket_srcs
    localsocketclient.cpp)

  add_library(mqlocalsocketclient ${mq_client_localsocket_srcs})
  list(APPEND localsocket_targets mqlocalsocketclient)
  list(APPEND localsocket_headers
    localsocketclient.h
    ${CMAKE_CURRENT_BINARY_DIR}/mqlocalsocketclientexport.h
  )
  set_target_properties(mqlocalsocketclient PROPERTIES AUTOMOC TRUE)
  target_link_libraries(mqlocalsocketclient
    molequeueclient
    mqlocalsocketconnection
  )

  generate_export_header(mqlocalsocketclient EXPORT_FILE_NAME mqlocalsocketclientexport.h)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_compiler_export_flags(molequeue_export_flags)
  set_property(TARGET mqlocalsocketclient APPEND
    PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

endif() # MQ_BUILD_CLIENT

install(FILES localsocket_headers
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/transport"
)
install(
  TARGETS
    ${localsocket_targets}
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )
add_subdirectory(plugin)
