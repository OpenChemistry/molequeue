set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)

add_library(ZeroMqServer MODULE zeromqconnectionlistenerfactory.cpp)
set_target_properties(ZeroMqServer PROPERTIES AUTOMOC TRUE)
target_link_libraries(ZeroMqServer MoleQueueZeroMqServer)

install(TARGETS ZeroMqServer DESTINATION lib/molequeue/plugins)
