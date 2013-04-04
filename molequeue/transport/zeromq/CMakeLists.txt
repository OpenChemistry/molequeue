find_package(ZeroMQ REQUIRED)

include_directories(${ZeroMQ_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DUSE_ZERO_MQ)

set(sources
  zeromqconnection.cpp
  zeromqconnectionlistener.cpp
)

set(headers
  zeromqconnection.h
  zeromqconnectionlistener.h
)

add_library(MoleQueueZeroMqServer STATIC ${sources})
set_target_properties(MoleQueueZeroMqServer PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueZeroMqServer
  MoleQueueServerPrivate
  ${QT_LIBRARIES}
  ${ZeroMQ_LIBRARIES}
)


install(FILES ${headers}
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/zeromqclient"
)

install(
  TARGETS MoleQueueZeroMqServer
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
)

add_subdirectory(plugin)
