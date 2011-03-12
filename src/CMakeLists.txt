find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt4_wrap_cpp(moc_srcs mainwindow.h)
qt4_wrap_ui(ui_srcs mainwindow.ui)
qt4_add_resources(rcc_srcs queuetray.qrc)

add_executable(QueueTray MACOSX_BUNDLE
  main.cpp mainwindow.cpp ${moc_srcs} ${ui_srcs} ${rcc_srcs})
target_link_libraries(QueueTray ${QT_LIBRARIES})
