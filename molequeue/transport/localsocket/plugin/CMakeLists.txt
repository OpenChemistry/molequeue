if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)
endif()

add_library(mqlocalsocketclf MODULE localsocketconnectionlistenerfactory.cpp)
set_target_properties(mqlocalsocketclf PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketclf mqlocalsocketconnectionlistener)

install(TARGETS mqlocalsocketclf DESTINATION lib/molequeue/plugins)
