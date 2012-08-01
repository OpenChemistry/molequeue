set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_BINARY_DIR}/lib/molequeue/plugins
  )

set(mq_zeromq_connectionlistenerfactory_srcs
  zeromqconnectionlistenerfactory.cpp
  )
set(mq_zeromq_connectionlistenerfactory_hdrs
  zeromqconnectionlistenerfactory.h
  )
qt4_wrap_cpp(mq_zeromq_connectionlistenerfactory_moc_srcs
  ${mq_zeromq_connectionlistenerfactory_hdrs}
  )

add_library(mqzeromqclf MODULE
  ${mq_zeromq_connectionlistenerfactory_srcs}
  ${mq_zeromq_connectionlistenerfactory_moc_srcs}
  )

target_link_libraries(mqzeromqclf
  mqzeromqconnectionlistener
  )

install(TARGETS mqzeromqclf DESTINATION lib/molequeue/plugins)
