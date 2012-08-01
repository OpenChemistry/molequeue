set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_BINARY_DIR}/lib/molequeue/plugins
  )

set(mq_localsocket_connectionlistenerfactory_srcs
  localsocketconnectionlistenerfactory.cpp
  )
set(mq_localsocket_connectionlistenerfactory_hdrs
  localsocketconnectionlistenerfactory.h
  )
qt4_wrap_cpp(mq_localsocket_connectionlistenerfactory_moc_srcs
  ${mq_localsocket_connectionlistenerfactory_hdrs}
  )

add_library(mqlocalsocketclf MODULE
  ${mq_localsocket_connectionlistenerfactory_srcs}
  ${mq_localsocket_connectionlistenerfactory_moc_srcs}
  )
target_link_libraries(mqlocalsocketclf
  mqlocalsocketconnectionlistener
  )

install(TARGETS mqlocalsocketclf DESTINATION lib/molequeue/plugins)
