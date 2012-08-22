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
  abstractqueuesettingswidget.cpp
  abstractrpcinterface.cpp
  actionfactorymanager.cpp
  addqueuedialog.cpp
  client.cpp
  importprogramdialog.cpp
  importqueuedialog.cpp
  job.cpp
  jobactionfactory.cpp
  jobactionfactories/killjobactionfactory.cpp
  jobactionfactories/opendirectoryactionfactory.cpp
  jobactionfactories/openwithactionfactory.cpp
  jobactionfactories/programmableopenwithactionfactory.cpp
  jobactionfactories/removejobactionfactory.cpp
  jobactionfactories/viewjoblogactionfactory.cpp
  jobdata.cpp
  jobitemmodel.cpp
  jobmanager.cpp
  jobreferencebase.cpp
  jobrequest.cpp
  jobtablewidget.cpp
  jobview.cpp
  jsonrpc.cpp
  localqueuewidget.cpp
  logentry.cpp
  logger.cpp
  logwindow.cpp
  mainwindow.cpp
  molequeueglobal.h
  openwithmanagerdialog.cpp
  openwithexecutablemodel.cpp
  openwithpatternmodel.cpp
  opensshcommand.cpp
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
  sshcommandfactory.cpp
  sshconnection.cpp
  templatekeyworddialog.cpp
  terminalprocess.cpp
)

if(WIN32)
  list(APPEND mq_srcs puttycommand.cpp)
endif()

qt4_wrap_ui(ui_srcs
  ui/addqueuedialog.ui
  ui/importprogramdialog.ui
  ui/importqueuedialog.ui
  ui/jobtablewidget.ui
  ui/localqueuewidget.ui
  ui/logwindow.ui
  ui/mainwindow.ui
  ui/openwithmanagerdialog.ui
  ui/programconfiguredialog.ui
  ui/queuemanagerdialog.ui
  ui/queuesettingsdialog.ui
  ui/remotequeuewidget.ui
  ui/templatekeyworddialog.ui
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
  ${ui_srcs}
  )
set_target_properties(molequeue_static PROPERTIES AUTOMOC TRUE)

target_link_libraries(molequeue_static
  mqconnection
  mqlocalsocketclient
  mqconnectionlistener)

add_executable(molequeue MACOSX_BUNDLE main.cpp ${rcc_srcs})
target_link_libraries(molequeue
  molequeue_static
  ${QT_LIBRARIES})

install(TARGETS molequeue
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  BUNDLE DESTINATION .
  )

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

set(mqclient_headers
  abstractrpcinterface.h
  client.h
  jobreferencebase.h
  jobrequest.h
  molequeueglobal.h
)

add_library(molequeueclient STATIC
  ${mqclient_srcs})
set_target_properties(molequeueclient PROPERTIES AUTOMOC TRUE)
target_link_libraries(molequeueclient
  ${QT_LIBRARIES}
  mqconnection)
install(FILES ${mqclient_headers}
  DESTINATION "${INSTALL_INCLUDE_DIR}/molequeue")

add_subdirectory(transport)

if(BUILD_TESTING)
  add_subdirectory(testing)
endif()


# Keep "add_subdirectory(lastinstall)" last: fixup_bundle needs to be
# *after* all other install(TARGETS and install(FILES calls
add_subdirectory(lastinstall)
