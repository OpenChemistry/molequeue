# Connection
set(mq_localsocket_connection_srcs
  localsocketconnection.cpp
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

install(FILES ${localsocket_headers}
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
