add_library(mqconnection
  connection.h
  connectionlistener.h
  jsonrpc.cpp
  message.cpp
  messageidmanager_p.cpp
)
set_target_properties(mqconnection PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqconnection
  ${QT_LIBRARIES}
  Qt5Json
)

set(hdrs
  connection.h
  connectionlistener.h
  connectionlistenerfactory.h
  jsonrpc.h
  message.h
)

generate_export_header(mqconnection EXPORT_FILE_NAME mqconnectionexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET mqconnection APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

list(APPEND hdrs
  "${CMAKE_CURRENT_BINARY_DIR}/mqconnectionexport.h")

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/transport")
install(TARGETS mqconnection
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
  )

add_subdirectory(localsocket)

# Are we using ZeroMQ
if(USE_ZERO_MQ)
  add_subdirectory(zeromq)
endif()
