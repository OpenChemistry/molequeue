set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)

find_package(ZeroMQ REQUIRED)
include_directories(SYSTEM ${ZeroMQ_INCLUDE_DIR})

add_library(ZeroMqServer MODULE zeromqconnectionlistenerfactory.cpp)
set_target_properties(ZeroMqServer PROPERTIES AUTOMOC TRUE PREFIX "")
target_link_libraries(ZeroMqServer MoleQueueZeroMq)

install(TARGETS ZeroMqServer
  DESTINATION ${INSTALL_LIBRARY_DIR}/molequeue/plugins)
