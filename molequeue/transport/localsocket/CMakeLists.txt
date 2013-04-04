set(sources
  localsocketconnection.cpp
  localsocketconnectionlistener.cpp
)

set(headers
  localsocketconnection.h
  localsocketconnectionlistener.h
)

add_library(MoleQueueServerCore STATIC ${sources})
set_target_properties(MoleQueueServerCore PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueServerCore
  MoleQueueServerPrivate
  ${QT_LIBRARIES}
)

install(FILES ${headers}
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/transport"
)

install(
  TARGETS MoleQueueServerCore
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
)

add_subdirectory(plugin)
