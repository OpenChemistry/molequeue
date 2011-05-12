find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(mq_srcs
  Connection.cpp
  mainwindow.cpp
  program.cpp
  ProgramItemModel.cpp
  queue.cpp
  QueueLocal.cpp
  QueueRemote.cpp
  QueueSGE.cpp
  QueueItemModel.cpp
  sshcommand.cpp
  sshconnection.cpp
  terminalprocess.cpp)

set(mq_moc
  Connection.h
  mainwindow.h
  ProgramItemModel.h
  queue.h
  QueueLocal.h
  QueueRemote.h
  QueueSGE.h
  QueueItemModel.h
  sshcommand.h
  sshconnection.h
  terminalprocess.h)

qt4_wrap_cpp(moc_srcs ${mq_moc})
qt4_wrap_ui(ui_srcs mainwindow.ui)
qt4_add_resources(rcc_srcs queuetray.qrc)

add_library(molequeue_static STATIC
  ${mq_srcs} ${moc_srcs} ${ui_srcs})

add_executable(molequeue MACOSX_BUNDLE main.cpp ${rcc_srcs})
target_link_libraries(molequeue molequeue_static ${QT_LIBRARIES})

add_subdirectory(testing)
