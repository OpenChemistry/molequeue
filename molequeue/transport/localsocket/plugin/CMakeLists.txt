set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_BINARY_DIR}/lib/molequeue/plugins
  )

add_library(mqlocalsocketclf MODULE localsocketconnectionlistenerfactory.cpp)
set_target_properties(mqlocalsocketclf PROPERTIES AUTOMOC TRUE)
target_link_libraries(mqlocalsocketclf mqlocalsocketconnectionlistener)

install(TARGETS mqlocalsocketclf DESTINATION lib/molequeue/plugins)
