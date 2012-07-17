set(mq_localsocket_connection_srcs
  localsocketconnection.cpp)

set(mq_localsocket_connection_hdrs
  localsocketconnection.h)

set(mq_localsocket_connectionlistener_srcs
  localsocketconnectionlistener.cpp)

set(mq_localsocket_connectionlistener_hdrs
  localsocketconnectionlistener.h)

set(mq_client_localsocket_srcs
  localsocketclient.cpp)

qt4_wrap_cpp(mq_localsocket_connection_moc_srcs
  ${mq_localsocket_connection_hdrs})

qt4_wrap_cpp(mq_localsocket_connectionlistener_moc_srcs
  ${mq_localsocket_connectionlistener_hdrs})

add_library(mqlocalsocketconnection STATIC
  ${mq_localsocket_connection_srcs}
  ${mq_localsocket_connection_moc_srcs})

target_link_libraries(mqlocalsocketconnection
  mqconnection
  ${QT_LIBRARIES})

add_library(mqlocalsocketconnectionlistener STATIC
  ${mq_localsocket_connectionlistener_srcs}
  ${mq_localsocket_connectionlistener_moc_srcs})

target_link_libraries(mqlocalsocketconnectionlistener
  mqconnectionlistener
  mqlocalsocketconnection
  ${QT_LIBRARIES})

# local socket client
add_library(mqlocalsocketclient
  ${mq_client_localsocket_srcs})
target_link_libraries(mqlocalsocketclient
  molequeueclient
  mqlocalsocketconnection)

set(hdrs localsocketclient.h ${CMAKE_CURRENT_BINARY_DIR}/mqlocalsocketclientexport.h)

generate_export_header(mqlocalsocketclient EXPORT_FILE_NAME mqlocalsocketclientexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET mqlocalsocketclient APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/localsocketclient")
install(TARGETS mqlocalsocketclient DESTINATION ${INSTALL_LIBRARY_DIR})

add_subdirectory(plugin)
