find_package(Qt5Network REQUIRED)
find_package(ZeroMQ REQUIRED)
include_directories(SYSTEM ${ZeroMQ_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(GenerateExportHeader)

add_definitions(-DUSE_ZERO_MQ)

set(sources
  zeromqconnection.cpp
  zeromqconnectionlistener.cpp
)

add_library(MoleQueueZeroMq ${sources})
qt5_use_modules(MoleQueueZeroMq Network)
set_target_properties(MoleQueueZeroMq PROPERTIES AUTOMOC TRUE)
target_link_libraries(MoleQueueZeroMq MoleQueueServerCore ${ZeroMQ_LIBRARIES})

generate_export_header(MoleQueueZeroMq
  EXPORT_FILE_NAME molequeuezeromqexport.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compiler_export_flags(molequeue_export_flags)
set_property(TARGET MoleQueueZeroMq APPEND
  PROPERTY COMPILE_FLAGS ${molequeue_export_flags})

list(APPEND hdrs
  "${CMAKE_CURRENT_BINARY_DIR}/molequeuezeromqexport.h")

install(FILES ${hdrs} DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue/zeromq")
install(TARGETS MoleQueueZeroMq
  EXPORT "MoleQueueTargets"
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})
