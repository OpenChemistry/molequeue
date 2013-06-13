find_package(Qt4 4.8 COMPONENTS QtCore QtNetwork REQUIRED)
include_directories(SYSTEM ${QT_INCLUDES})
add_definitions(${QT_DEFINITIONS})

add_subdirectory(localsocketserver)

if(USE_ZERO_MQ)
  add_subdirectory(zeromqserver)
endif()
