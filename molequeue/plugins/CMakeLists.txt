find_package(Qt5Network REQUIRED)

if (MSVC)
  # Do not generate manifests for the plugins - caused issues loading plugins
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif()

set_property(GLOBAL PROPERTY MoleQueue_PLUGINS)

add_subdirectory(localsocketserver)

if(USE_ZERO_MQ)
  add_subdirectory(zeromqserver)
endif()
