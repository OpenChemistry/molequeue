find_package(Qt4 4.8.0 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
include(GenerateExportHeader)

set(JSONCPP_SOURCE_DIR "${MoleQueue_SOURCE_DIR}/thirdparty/jsoncpp/")

include_directories(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${JSONCPP_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/transport
  ${CMAKE_CURRENT_BINARY_DIR}/transport/localsocket)

# multi configuration build? Needed for plugin search path
if(CMAKE_CONFIGURATION_TYPES)
  add_definitions(-DMULTI_CONFIG_BUILD)
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(mq_srcs
  abstractrpcinterface.cpp
  actionfactorymanager.cpp
  addqueuedialog.cpp
  client.cpp
  job.cpp
  jobactionfactory.cpp
  jobactionfactories/opendirectoryactionfactory.cpp
  jobactionfactories/openwithactionfactory.cpp
  jobactionfactories/programmableopenwithactionfactory.cpp
  jobactionfactories/removejobactionfactory.cpp
  jobdata.cpp
  jobitemmodel.cpp
  jobmanager.cpp
  jobreferencebase.cpp
  jobrequest.cpp
  jobtablewidget.cpp
  jobview.cpp
  jsonrpc.cpp
  logentry.cpp
  logger.cpp
  logwindow.cpp
  mainwindow.cpp
  molequeueglobal.h
  openwithmanagerdialog.cpp
  openwithexecutablemodel.cpp
  openwithpatternmodel.cpp
  patterntypedelegate.cpp
  pluginmanager.cpp
  program.cpp
  programconfiguredialog.cpp
  queue.cpp
  queuemanager.cpp
  queuemanagerdialog.cpp
  queuemanageritemmodel.cpp
  queueprogramitemmodel.cpp
  queues/local.cpp
  queues/pbs.cpp
  queues/remote.cpp
  queues/sge.cpp
  queuesettingsdialog.cpp
  remotequeuewidget.cpp
  server.cpp
  sshcommand.cpp
  sshconnection.cpp
  terminalprocess.cpp
)

set(mq_moc
  abstractrpcinterface.h
  actionfactorymanager.h
  addqueuedialog.h
  client.h
  jobactionfactory.h
  jobactionfactories/opendirectoryactionfactory.h
  jobactionfactories/openwithactionfactory.h
  jobactionfactories/programmableopenwithactionfactory.h
  jobactionfactories/removejobactionfactory.h
  jobitemmodel.h
  jobmanager.h
  jobtablewidget.h
  jobview.h
  jsonrpc.h
  logger.h
  logwindow.h
  mainwindow.h
  openwithmanagerdialog.h
  openwithexecutablemodel.h
  openwithpatternmodel.h
  patterntypedelegate.h
  pluginmanager.h
  program.h
  programconfiguredialog.h
  queue.h
  queuemanager.h
  queuemanagerdialog.h
  queuemanageritemmodel.h
  queueprogramitemmodel.h
  queues/local.h
  queues/pbs.h
  queues/remote.h
  queues/sge.h
  queuesettingsdialog.h
  remotequeuewidget.h
  server.h
  sshcommand.h
  sshconnection.h
  terminalprocess.h
)

qt4_wrap_cpp(moc_srcs ${mq_moc})

qt4_wrap_ui(ui_srcs
  ui/addqueuedialog.ui
  ui/jobtablewidget.ui
  ui/logwindow.ui
  ui/mainwindow.ui
  ui/openwithmanagerdialog.ui
  ui/programconfiguredialog.ui
  ui/queuemanagerdialog.ui
  ui/queuesettingsdialog.ui
  ui/remotequeuewidget.ui
)

qt4_add_resources(rcc_srcs queuetray.qrc)

# Pull in JsonCpp
aux_source_directory(${JSONCPP_SOURCE_DIR} jsoncpp_srcs)
set(mq_srcs ${mq_srcs} ${jsoncpp_srcs})

# Disable JsonCpp warnings for most platforms
if(BORLAND)
  set_property(SOURCE ${jsoncpp_srcs}
    PROPERTY
    COMPILE_FLAGS "-w-")
else()
  set_property(SOURCE ${jsoncpp_srcs}
    PROPERTY
    COMPILE_FLAGS "-w")
endif()

add_library(molequeue_static STATIC
  ${mq_srcs}
  ${moc_srcs}
  ${ui_srcs}
  )

target_link_libraries(molequeue_static
  mqconnection
  mqlocalsocketclient
  mqconnectionlistener)

add_executable(molequeue MACOSX_BUNDLE main.cpp ${rcc_srcs})
target_link_libraries(molequeue
  molequeue_static
  ${QT_LIBRARIES})

install(TARGETS molequeue
  DESTINATION ${INSTALL_RUNTIME_DIR})

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

# Client library
set(mqclient_srcs
  abstractrpcinterface.cpp
  client.cpp
  job.cpp
  jobdata.cpp
  jobitemmodel.cpp
  jobmanager.cpp
  jobreferencebase.cpp
  jobrequest.cpp
  jsonrpc.cpp
  logentry.cpp
  logger.cpp
  molequeueglobal.h
  ${jsoncpp_srcs}
)

set(mqclient_moc
  abstractrpcinterface.h
  client.h
  jobitemmodel.h
  jobmanager.h
  jsonrpc.h
  logger.h
)

#TODO need to install these
set(mqclient_headers
  abstractrpcinterface.h
  client.h
  jobrequest.h
  molequeueglobal.h
  ${JSONCPP_SOURCE_DIR}/json/json-forwards.h
)


qt4_wrap_cpp(mqclient_moc_srcs ${mqclient_moc})
add_library(molequeueclient STATIC
  ${mqclient_srcs}
  ${mqclient_moc_srcs})
target_link_libraries(molequeueclient
  ${QT_LIBRARIES}
  mqconnection)
install(TARGETS molequeueclient DESTINATION ${INSTALL_LIBRARY_DIR})

add_subdirectory(transport)

if(BUILD_TESTING)
  add_subdirectory(testing)
endif()
