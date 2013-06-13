find_package(Qt4 4.8 COMPONENTS QtCore QtNetwork REQUIRED)
add_definitions(${QT_DEFINITIONS})
find_package(ZeroMQ REQUIRED)
include_directories(SYSTEM ${QT_INCLUDES} ${ZeroMQ_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DUSE_ZERO_MQ)

set(sources
  zeromqconnection.cpp
  zeromqconnectionlistener.cpp
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)

add_library(ZeroMqServer MODULE zeromqconnectionlistenerfactory.cpp ${sources})
set_target_properties(ZeroMqServer PROPERTIES AUTOMOC TRUE)
target_link_libraries(ZeroMqServer
  MoleQueueServerCore
  ${QT_LIBRARIES}
  ${ZeroMQ_LIBRARIES})

install(TARGETS ZeroMqServer DESTINATION lib/molequeue/plugins)
