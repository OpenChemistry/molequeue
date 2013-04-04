add_library(MoleQueueServerPrivate
  connection.h
  connectionlistener.h
  jsonrpc.cpp
  message.cpp
  messageidmanager_p.cpp
)
set_target_properties(MoleQueueServerPrivate PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueServerPrivate
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

generate_export_header(MoleQueueServerPrivate
  EXPORT_FILE_NAME molequeueserverprivateexport.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET MoleQueueServerPrivate APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

list(APPEND hdrs
  "${CMAKE_CURRENT_BINARY_DIR}/molequeueserverprivateexport.h")

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/transport")
install(TARGETS MoleQueueServerPrivate
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
