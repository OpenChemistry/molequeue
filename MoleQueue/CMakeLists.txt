find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(mq_srcs
  addqueuedialog.cpp
  Connection.cpp
  job.cpp
  mainwindow.cpp
  program.cpp
  ProgramItemModel.cpp
  ProgramTableView.cpp
  queue.cpp
  QueueLocal.cpp
  QueueRemote.cpp
  queuemanager.cpp
  queuemanagerdialog.cpp
  queuesettingsdialog.cpp
  QueueSGE.cpp
  QueueItemModel.cpp
  sshcommand.cpp
  sshconnection.cpp
  terminalprocess.cpp)

set(mq_moc
  addqueuedialog.h
  Connection.h
  job.h
  mainwindow.h
  ProgramItemModel.h
  ProgramTableView.h
  queue.h
  QueueLocal.h
  QueueRemote.h
  queuemanager.h
  queuemanagerdialog.h
  queuesettingsdialog.h
  QueueSGE.h
  QueueItemModel.h
  sshcommand.h
  sshconnection.h
  terminalprocess.h)

qt4_wrap_cpp(moc_srcs ${mq_moc})
qt4_wrap_ui(ui_srcs
  addqueuedialog.ui
  mainwindow.ui
  queuemanagerdialog.ui
  queuesettingsdialog.ui
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
