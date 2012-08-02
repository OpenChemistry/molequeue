set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_BINARY_DIR}/lib/molequeue/plugins
  )

add_library(mqzeromqclf MODULE zeromqconnectionlistenerfactory.cpp)
set_target_properties(mqzeromqclf PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqzeromqclf
  mqzeromqconnectionlistener)

install(TARGETS mqzeromqclf DESTINATION lib/molequeue/plugins)
