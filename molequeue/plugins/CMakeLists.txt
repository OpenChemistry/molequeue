find_package(Qt4 4.8 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

if (MSVC)
  # Do not generate manifests for the plugins - caused issues loading plugins
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif()

set_property(GLOBAL PROPERTY MoleQueue_PLUGINS)

add_subdirectory(localsocketserver)

if(USE_ZERO_MQ)
  add_subdirectory(zeromqserver)
endif()
