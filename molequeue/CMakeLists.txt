find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(mq_srcs
  connection.cpp
  job.cpp
  jobitemmodel.cpp
  jobtreeview.cpp
  program.cpp
  queue.cpp
  queuemanager.cpp
  queueitemmodel.cpp
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
  connection.h
  job.h
  jobitemmodel.h
  jobtreeview.h
  queue.h
  queuemanager.h
  queueitemmodel.h
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

  include(DeployQt4)
  install_qt4_executable(bin/molequeue${sfx})
endif()

add_subdirectory(testing)
