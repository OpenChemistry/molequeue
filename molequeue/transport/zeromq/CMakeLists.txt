find_package(ZeroMQ REQUIRED)

include_directories(${ZeroMQ_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DUSE_ZERO_MQ)

# Connection
add_library(mqzeromqconnection STATIC
  zeromqconnection.cpp
  ${jsoncpp_srcs}
)
list(APPEND zeromq_targets mqzeromqconnection)
list(APPEND zeromq_headers zeromqconnection.h)
set_target_properties(mqzeromqconnection PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqzeromqconnection
  mqconnection
  ${QT_LIBRARIES}
  ${ZeroMQ_LIBRARIES})

# Connection Listener
add_library(mqzeromqconnectionlistener STATIC zeromqconnectionlistener.cpp)
list(APPEND zeromq_targets mqzeromqconnectionlistener)
list(APPEND zeromq_headers zeromqconnectionlistener.h)
set_target_properties(mqzeromqconnectionlistener PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqzeromqconnectionlistener
  mqconnectionlistener
  mqzeromqconnection)

# Client
if(MoleQueue_BUILD_CLIENT)
  add_library(mqzeromqclient zeromqclient.cpp)
  list(APPEND zeromq_targets mqzeromqclient)
  list(APPEND zeromq_headers
    zeromqclient.h
    ${CMAKE_CURRENT_BINARY_DIR}/mqzeromqclientexport.h
  )
  target_link_libraries(mqzeromqclient
    molequeueclient
    mqzeromqconnection
  )
  generate_export_header(mqzeromqclient EXPORT_FILE_NAME mqzeromqclientexport.h)

endif() # MoleQueue_BUILD_CLIENT

install(FILES ${zeromq_headers}
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/zeromqclient"
)

install(TARGETS
    ${zeromq_targets}
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
)

add_subdirectory(plugin)
