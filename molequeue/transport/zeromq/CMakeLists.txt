find_package(ZeroMQ REQUIRED)

include_directories(${ZeroMQ_INCLUDE_DIR})

add_definitions(-DUSE_ZERO_MQ)

add_library(mqzeromqconnection STATIC zeromqconnection.cpp)
set_target_properties(mqzeromqconnection PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqzeromqconnection
  mqconnection
  ${QT_LIBRARIES}
  ${ZeroMQ_LIBRARIES})

add_library(mqzeromqconnectionlistener STATIC zeromqconnectionlistener.cpp)
set_target_properties(mqzeromqconnectionlistener PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqzeromqconnectionlistener
  mqconnectionlistener
  mqzeromqconnection)

# zero client
add_library(mqzeromqclient zeromqclient.cpp)
target_link_libraries(mqzeromqclient
  molequeueclient
  mqzeromqconnection)

set(hdrs zeromqclient.h ${CMAKE_CURRENT_BINARY_DIR}/mqzeromqclientexport.h)

generate_export_header(mqzeromqclient EXPORT_FILE_NAME mqzeromqclientexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/zeromqclient")
install(TARGETS mqzeromqclient
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )

add_subdirectory(plugin)
