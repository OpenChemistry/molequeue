find_package(ZeroMQ REQUIRED)

include_directories(${ZeroMQ_INCLUDE_DIR})

message("${ZeroMQ_LIBRARIES}")

set(mq_zeromq_connection_srcs
  zeromqconnection.cpp
  )
set(mq_zeromq_connection_hdrs
  zeromqconnection.h
  )

set(mq_zeromq_connectionlistener_srcs
  zeromqconnectionlistener.cpp
  )
set(mq_zeromq_connectionlistener_hdrs
  zeromqconnectionlistener.h
  )

set(mq_client_zeromq_srcs
  zeromqclient.cpp
  )

add_definitions(-DUSE_ZERO_MQ)

qt4_wrap_cpp(mq_zeromq_connection_moc_srcs
  ${mq_zeromq_connection_hdrs}
  )
qt4_wrap_cpp(mq_zeromq_connectionlistener_moc_srcs
  ${mq_zeromq_connectionlistener_hdrs}
  )

add_library(mqzeromqconnection STATIC
  ${mq_zeromq_connection_srcs}
  ${mq_zeromq_connection_moc_srcs}
  )

target_link_libraries(mqzeromqconnection
  mqconnection
  ${QT_LIBRARIES}
  ${ZeroMQ_LIBRARIES}
)

add_library(mqzeromqconnectionlistener STATIC
  ${mq_zeromq_connectionlistener_srcs}
  ${mq_zeromq_connectionlistener_moc_srcs}
  )

target_link_libraries(mqzeromqconnectionlistener
  mqconnectionlistener
  mqzeromqconnection
)

# zero client
add_library(mqzeromqclient
  ${mq_client_zeromq_srcs}
  )
target_link_libraries(mqzeromqclient
  molequeueclient
  mqzeromqconnection
  )

set(hdrs mqzeromqclient.h mqzeromqclientexport.h)

generate_export_header(mqzeromqclient EXPORT_FILE_NAME mqzeromqclientexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/zeromqclient")
install(TARGETS mqzeromqclient DESTINATION lib)

add_subdirectory(plugin)
