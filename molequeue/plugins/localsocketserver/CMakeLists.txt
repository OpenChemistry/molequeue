set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)

add_library(LocalSocketServer MODULE localsocketconnectionlistenerfactory.cpp)
set_target_properties(LocalSocketServer PROPERTIES AUTOMOC TRUE PREFIX "")
target_link_libraries(LocalSocketServer MoleQueueServerCore)

install(TARGETS LocalSocketServer
  DESTINATION ${INSTALL_LIBRARY_DIR}/molequeue/plugins)
