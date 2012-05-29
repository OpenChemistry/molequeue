find_package(Qt4 4.8.0 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})

set(JSONCPP_SOURCE_DIR "${MoleQueue_SOURCE_DIR}/thirdparty/jsoncpp/")

include_directories(${CMAKE_CURRENT_BINARY_DIR}
  ${JSONCPP_SOURCE_DIR}
)

set(mq_srcs
  abstractrpcinterface.cpp
  client.cpp
  connection.cpp
  job.cpp
  jobitemmodel.cpp
  jobmanager.cpp
  jobview.cpp
  jsonrpc.cpp
  molequeueglobal.h
  program.cpp
  queue.cpp
  queuemanager.cpp
  queueitemmodel.cpp
  server.cpp
  serverconnection.cpp
  sshcommand.cpp
  sshconnection.cpp
  terminalprocess.cpp
  queues/local.cpp
  queues/remote.cpp
  queues/sge.cpp
  addqueuedialog.cpp
  mainwindow.cpp
  queuemanagerdialog.cpp
  queuesettingsdialog.cpp
)

set(mq_moc
  abstractrpcinterface.h
  client.h
  connection.h
  jobitemmodel.h
  jobmanager.h
  jobview.h
  jsonrpc.h
  program.h
  queue.h
  queuemanager.h
  queueitemmodel.h
  server.h
  serverconnection.h
  sshcommand.h
  sshconnection.h
  terminalprocess.h
  queues/local.h
  queues/remote.h
  queues/sge.h
  addqueuedialog.h
  mainwindow.h
  queuemanagerdialog.h
  queuesettingsdialog.h
)
qt4_wrap_cpp(moc_srcs ${mq_moc})

qt4_wrap_ui(ui_srcs
  ui/addqueuedialog.ui
  ui/mainwindow.ui
  ui/queuemanagerdialog.ui
  ui/queuesettingsdialog.ui
)
qt4_add_resources(rcc_srcs queuetray.qrc)

# Pull in JsonCpp
aux_source_directory(${JSONCPP_SOURCE_DIR} jsoncpp_srcs)
set(mq_srcs ${mq_srcs} ${jsoncpp_srcs})

add_library(molequeue_static STATIC
  ${mq_srcs} ${moc_srcs} ${ui_srcs})

add_executable(molequeue MACOSX_BUNDLE main.cpp ${rcc_srcs})
target_link_libraries(molequeue molequeue_static ${QT_LIBRARIES})
install(TARGETS molequeue
  DESTINATION bin)

if((APPLE OR WIN32) AND NOT ${CMAKE_VERSION} VERSION_LESS 2.8.8)
  set(sfx "")
  if(APPLE)
    set(sfx ".app")
  elseif(WIN32)
    set(sfx ".exe")
  endif()

  set(dirs "")
  if(CMAKE_PREFIX_PATH)
    set(dirs "${CMAKE_PREFIX_PATH}/lib")
  endif()

  include(DeployQt4)
  install_qt4_executable(bin/molequeue${sfx} "" "" "${dirs}")
endif()

if(ENABLE_TESTING)
  add_subdirectory(testing)
endif()

# Client library
set(mqclient_srcs
  abstractrpcinterface.cpp
  client.cpp
  job.cpp
  jobmanager.cpp
  jsonrpc.cpp
  molequeueglobal.h
  ${jsoncpp_srcs}
)

set(mqclient_moc
  abstractrpcinterface.h
  client.h
  jobmanager.h
  jsonrpc.h
)

#TODO need to install these
set(mqclient_headers
  abstractrpcinterface.h
  client.h
  job.h
  molequeueglobal.h
  ${JSONCPP_SOURCE_DIR}/json/json-forwards.h
)

qt4_wrap_cpp(mqclient_moc_srcs ${mqclient_moc})
add_library(molequeueclient SHARED ${mqclient_srcs} ${mqclient_moc_srcs})
target_link_libraries(molequeueclient ${QT_LIBRARIES})
install(TARGETS molequeueclient DESTINATION lib)
