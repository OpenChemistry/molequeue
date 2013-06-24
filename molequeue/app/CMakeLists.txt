set(qt_components QtCore QtGui QtNetwork)

if(MoleQueue_USE_EZHPC_UIT)
  list(APPEND qt_components QtXmlPatterns)
endif()
if(WIN32)
  list(APPEND qt_components QtMain)
endif()

find_package(Qt4 4.8.0 COMPONENTS ${qt_components} REQUIRED)
include(${QT_USE_FILE})

# Find a python 2.x interpreter.
find_package(PythonInterp 2 QUIET)

include(GenerateExportHeader)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/transport
  ${CMAKE_CURRENT_BINARY_DIR}/transport/localsocket)

# multi configuration build? Needed for plugin search path
if(CMAKE_CONFIGURATION_TYPES)
  add_definitions(-DMULTI_CONFIG_BUILD)
endif()

set(mq_srcs
  aboutdialog.cpp
  abstractqueuesettingswidget.cpp
  advancedfilterdialog.cpp
  actionfactorymanager.cpp
  addqueuedialog.cpp
  filebrowsewidget.cpp
  filespecification.cpp
  filesystemtools.cpp
  importprogramdialog.cpp
  importqueuedialog.cpp
  job.cpp
  jobactionfactory.cpp
  jobactionfactories/killjobactionfactory.cpp
  jobactionfactories/opendirectoryactionfactory.cpp
  jobactionfactories/openwithactionfactory.cpp
  jobactionfactories/removejobactionfactory.cpp
  jobactionfactories/viewjoblogactionfactory.cpp
  jobdata.cpp
  jobitemmodel.cpp
  jobmanager.cpp
  jobreferencebase.cpp
  jobtableproxymodel.cpp
  jobtablewidget.cpp
  jobview.cpp
  localqueuewidget.cpp
  logentry.cpp
  logger.cpp
  logwindow.cpp
  mainwindow.cpp
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
  queues/remotessh.cpp
  queues/sge.cpp
  queues/slurm.cpp
  queuesettingsdialog.cpp
  remotequeuewidget.cpp
  server.cpp
  sshcommand.cpp
  sshcommandfactory.cpp
  sshconnection.cpp
  templatekeyworddialog.cpp
  terminalprocess.cpp)

set(ui_files
  ui/aboutdialog.ui
  ui/addqueuedialog.ui
  ui/advancedfilterdialog.ui
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
  ui/templatekeyworddialog.ui)

if(MoleQueue_USE_EZHPC_UIT)

  find_package(KDSoap REQUIRED)
  include_directories(${KDSoap_INCLUDE_DIRS})
  kdsoap_generate_wsdl(ezHPC_UIT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/wsdl/uitapi.wsdl)

  list(APPEND mq_srcs
    ${ezHPC_UIT_SRCS}
    credentialsdialog.cpp
    queues/uit/authenticatecont.cpp
    queues/uit/authenticateresponse.cpp
    queues/uit/authresponseprocessor.cpp
    queues/uit/compositeiodevice.cpp
    queues/uit/dirlistinginfo.cpp
    queues/uit/fileinfo.cpp
    queues/uit/filestreamingdata.cpp
    queues/uit/jobevent.cpp
    queues/uit/jobeventlist.cpp
    queues/uit/jobsubmissioninfo.cpp
    queues/uit/kerberoscredentials.cpp
    queues/uit/messagehandler.cpp
    queues/queueuit.cpp
    queues/uit/sslsetup.cpp
    queues/uit/authenticator.cpp
    queues/uit/directoryupload.cpp
    queues/uit/directorycreate.cpp
    queues/uit/directorydelete.cpp
    queues/uit/directorydownload.cpp
    queues/uit/filesystemoperation.cpp
    queues/uit/requests.cpp
    queues/uit/session.cpp
    queues/uit/sessionmanager.cpp
    queues/uit/userhostassoc.cpp
    queues/uit/userhostassoclist.cpp
    uitqueuewidget.cpp
    wsdl_uitapi.cpp)

  list(APPEND ui_files
    ui/uitqueuewidget.ui
    ui/credentialsdialog.ui)

  # Disable warnings for KDSoap generate file for most platforms
  silence_warnings(wsdl_uitapi.cpp)

  # install the SSL certificates
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/certs
    DESTINATION ${INSTALL_DATA_DIR}/molequeue)

  # copy to build tree so things will work when running from build tree
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/certs
    DESTINATION  ${CMAKE_BINARY_DIR}/${INSTALL_DATA_DIR}/molequeue)

  set(MoleQueue_SSL_CERT_DIR "${INSTALL_DATA_DIR}/molequeue/certs")
endif()

if(WIN32)
  list(APPEND mq_srcs puttycommand.cpp)
endif()

qt4_wrap_ui(ui_srcs ${ui_files})

qt4_add_resources(rcc_srcs queuetray.qrc)

# Use an object library for the bulk of the code in MoleQueue, this allows us
# to reuse the objects in testing without additional work for the primary use
# of the code in the application.
add_library(MoleQueueObjects OBJECT ${mq_srcs} ${ui_srcs})
set_target_properties(MoleQueueObjects PROPERTIES AUTOMOC TRUE)
set(MoleQueue_LINK_LIBRARIES MoleQueueServerCore ${QT_LIBRARIES})
if(MoleQueue_USE_EZHPC_UIT)
  list(APPEND MoleQueue_LINK_LIBRARIES kdsoap)
endif()
if(MoleQueue_BUILD_CLIENT)
  list(APPEND MoleQueue_LINK_LIBRARIES MoleQueueClient)
endif()

set(sources main.cpp)
# Handle Mac OS X specific icons etc.
if(APPLE)
  list(APPEND sources icons/molequeue.icns)
  set(MACOSX_BUNDLE_ICON_FILE molequeue.icns)
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${MoleQueue_VERSION}")
  set_source_files_properties(icons/molequeue.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32)
  list(APPEND sources icons/molequeue.rc)
endif()

add_executable(molequeue WIN32 MACOSX_BUNDLE ${sources} ${rcc_srcs}
  $<TARGET_OBJECTS:MoleQueueObjects>)
target_link_libraries(molequeue ${MoleQueue_LINK_LIBRARIES})
if(APPLE)
  set_target_properties(molequeue PROPERTIES OUTPUT_NAME ${MACOSX_BUNDLE_NAME})
endif()

install(TARGETS molequeue
  RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
  BUNDLE DESTINATION .
  )

include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}/client" # For export macro
  "${MoleQueue_SOURCE_DIR}/thirdparty/qt5json/") # For Qt5JSON

# Config file for build options.
configure_file(molequeueconfig.h.in molequeueconfig.h)

# Only run tests if building both client and app:
if(ENABLE_TESTING AND MoleQueue_BUILD_CLIENT)
  add_subdirectory(testing)
endif()
