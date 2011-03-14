find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(mq_srcs
  main.cpp
  mainwindow.cpp
  sshcommand.cpp
  sshconnection.cpp
  terminalprocess.cpp)

set(mq_moc
  mainwindow.h
  sshcommand.h
  sshconnection.h
  terminalprocess.h)

qt4_wrap_cpp(moc_srcs ${mq_moc})
qt4_wrap_ui(ui_srcs mainwindow.ui)
qt4_add_resources(rcc_srcs queuetray.qrc)

add_executable(molequeue MACOSX_BUNDLE
  ${mq_srcs} ${moc_srcs} ${ui_srcs} ${rcc_srcs})
target_link_libraries(molequeue ${QT_LIBRARIES})
