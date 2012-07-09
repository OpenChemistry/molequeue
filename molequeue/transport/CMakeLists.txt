set(mq_connection_hdrs
  connection.h)

set(mq_connectionlistener_hdrs
  connectionlistener.h)

qt4_wrap_cpp(mq_connection_moc_srcs ${mq_connection_hdrs})
qt4_wrap_cpp(mq_connectionlistener_moc_srcs ${mq_connectionlistener_hdrs})

add_library(mqconnection
  ${mq_connection_moc_srcs}
  message.cpp)

target_link_libraries(mqconnection
  ${QT_LIBRARIES})

add_library(mqconnectionlistener
  ${mq_connectionlistener_moc_srcs})

target_link_libraries(mqconnectionlistener
  ${QT_LIBRARIES})

set(hdrs
  ${mq_connection_hdrs}
  ${mq_connectionlistener_hdrs}
  connectionlistenerfactory.h)

generate_export_header(mqconnection EXPORT_FILE_NAME mqconnectionexport.h)
generate_export_header(mqconnectionlistener EXPORT_FILE_NAME mqconnectionlistenerexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET mqconnection APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})
set_property(TARGET mqconnectionlistener APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

list(APPEND hdrs
  mqconnectionexport.h
  mqconnectionlistenerexport.h)

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue")

add_subdirectory(localsocket)

# Are we using ZeroMQ
if(USE_ZERO_MQ)
  add_subdirectory(zeromq)
endif()