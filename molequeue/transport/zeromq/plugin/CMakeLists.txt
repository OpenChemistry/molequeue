set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/molequeue/plugins)

add_library(mqzeromqclf MODULE zeromqconnectionlistenerfactory.cpp)
set_target_properties(mqzeromqclf PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqzeromqclf
  mqzeromqconnectionlistener)

install(TARGETS mqzeromqclf DESTINATION lib/molequeue/plugins)
